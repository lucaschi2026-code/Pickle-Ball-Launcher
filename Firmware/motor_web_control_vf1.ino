/*
 * ESP32 web control -- two BTS7960 drive motors + two TMC2209 steppers
 * --------------------------------------------------------------------
 * Board:  ESP32 Dev Module (WROOM-32)
 * Core:   arduino-esp32 v3.x   (v2.x alternatives noted at each API call)
 *
 * Connect phone to WiFi "RoverControl", password "rover1234",
 * then open  http://192.168.4.1
 *
 * DRIVE MOTORS -- forward only.
 *   The two motors face opposite ways, so they run on opposite half-bridges:
 *   motor 1 on RPWM, motor 2 on LPWM. Each motor's other pin is held at 0,
 *   so no input can command reverse.
 *
 * STEPPERS -- hold-to-move jog control.
 *   Stepper 1 = TILT   (3.33:1 spur reduction, deliberately slow)
 *   Stepper 2 = PAN    (400:1 worm, self-locking, needs a high motor rate
 *                       to produce any useful platform speed)
 *   Step pulses come from a 20 kHz hardware timer interrupt, NOT from loop().
 *   That matters: loop() also runs the web server, which stalls for tens of
 *   milliseconds at a time. Generating steps there would give visibly jerky
 *   motion and skipped steps.
 *
 * SAFETY
 *  - Everything DISARMED at boot. You must press Arm.
 *  - If the phone stops sending updates for 1.5 s, everything stops.
 *  - TMC2209 EN is ACTIVE LOW (opposite to the BTS7960s). Fit a 10k pull-up
 *    from D21 to 3.3V so the steppers stay disabled while the ESP32 boots.
 *  - Not a substitute for the physical switch or the fuses.
 */

#include <WiFi.h>
#include <WebServer.h>
#include "soc/gpio_reg.h"   // GPIO_OUT_W1TS_REG / W1TC_REG for fast step pulses

// ==================== pins ====================
// drive motors
const int M1_RPWM = 26;
const int M1_LPWM = 27;
const int M2_RPWM = 32;
const int M2_LPWM = 33;
const int EN_PIN  = 13;   // both BTS7960 R_EN + L_EN, ACTIVE HIGH

// steppers
// Tilt is on D25/D14 rather than D18/D19: D18 was proven dead on this board
// (same wire and same driver worked immediately on D25). D19 was never
// shown to be faulty -- a dead DIR pin still produces motion -- but it is
// left unused since D18 sits next to it and the cause is unknown.
//
// Note: D14 glitches briefly at boot on the ESP32. Harmless here, because
// it carries DIR (direction only) and TMC_EN holds the drivers disabled
// until setup() runs. Do not put STEP on it.
const int TILT_STEP = 25;
const int TILT_DIR  = 14;
const int PAN_STEP  = 22;
const int PAN_DIR   = 23;
const int TMC_EN    = 21;   // both TMC2209 EN, ACTIVE LOW

// Which half-bridge each drive motor uses. Motor 2 is mirrored.
// Swap a motor's pair here if it turns the wrong way.
const int M1_DRIVE = M1_RPWM;
const int M1_IDLE  = M1_LPWM;
const int M2_DRIVE = M2_LPWM;
const int M2_IDLE  = M2_RPWM;

// ==================== tuning ====================
const int PWM_FREQ = 10000;   // 10 kHz
const int PWM_BITS = 8;       // 0-255

/*  Stepper speeds, in motor microsteps per second.
 *  Assumes 1/8 microstepping: 200 full steps x 8 = 1600 microsteps per
 *  motor revolution.
 *
 *  TILT -- 3.33:1
 *    1600 x 3.33 = 5328 microsteps per degree-of-arc revolution
 *    400 steps/s  ->  0.25 motor rev/s  ->  about 27 deg/s at the arm
 *
 *  PAN -- 400:1 worm
 *    1600 x 400 = 640,000 microsteps per platform revolution
 *    8000 steps/s -> 5 motor rev/s (300 rpm) -> about 4.5 deg/s at the platform
 *
 *  The huge difference is entirely the gearing. Pan needs a fast motor to
 *  move the platform at all; tilt needs a slow one to stay controllable.
 *
 *  If pan stalls or screeches, lower PAN_MAX_SPS. A NEMA 17 on 11.1V starts
 *  losing torque badly somewhere around 300-400 rpm.
 */
const int32_t TILT_MAX_SPS = 400;
const int32_t PAN_MAX_SPS  = 8000;

// Acceleration, steps/s^2. Steppers cannot start at full speed from rest --
// without a ramp the motor just buzzes and skips.
const int32_t TILT_ACCEL = 1500;    // reaches full speed in ~0.27 s
const int32_t PAN_ACCEL  = 16000;   // reaches full speed in ~0.50 s

// Set true if an axis jogs the wrong way. Easier than resoldering.
const bool TILT_INVERT = false;
const bool PAN_INVERT  = true;    // left/right were reversed on this build

// ==================== network ====================
const char* AP_SSID = "RoverControl";
const char* AP_PASS = "rover1234";   // min 8 chars

WebServer server(80);

// ==================== state ====================
int  speed1 = 0;              // 0..255, forward only
int  speed2 = 0;
bool armed  = false;
unsigned long lastCommand = 0;
unsigned long lastRamp    = 0;
const unsigned long TIMEOUT_MS = 1500;

// ==================== stepper engine ====================
/*  Direct digital synthesis: a fixed-rate timer adds `inc` to an accumulator
 *  every tick. Each time the accumulator crosses 65536 it emits one step
 *  pulse and subtracts 65536. Changing `inc` changes the step rate smoothly,
 *  with no timer reconfiguration and no drift.
 */
const uint32_t TICK_HZ = 20000;          // ISR rate -> 20,000 steps/s ceiling

struct Axis {
  uint8_t  stepPin;
  uint8_t  dirPin;
  bool     invert;
  int32_t  maxRate;              // steps/s
  int32_t  accel;                // steps/s^2
  volatile uint32_t acc;         // DDS accumulator
  volatile uint32_t inc;         // DDS increment (written by loop, read by ISR)
  int32_t  rate;                 // current rate magnitude, steps/s
  int8_t   cmdDir;               // -1, 0, +1 requested by the UI
  int8_t   curDir;               // -1 or +1 currently set on the DIR pin
};

Axis axTilt = { TILT_STEP, TILT_DIR, TILT_INVERT, TILT_MAX_SPS, TILT_ACCEL, 0, 0, 0, 0, 1 };
Axis axPan  = { PAN_STEP,  PAN_DIR,  PAN_INVERT,  PAN_MAX_SPS,  PAN_ACCEL,  0, 0, 0, 0, 1 };

hw_timer_t* stepTimer = nullptr;

// All STEP pins, as a register bit mask.
static const uint32_t STEP_MASK = (1UL << TILT_STEP) | (1UL << PAN_STEP);

/*  Pulse generation.
 *  A STEP pulse is started on the tick that fires it and cleared at the top
 *  of the NEXT tick, giving a 50 us high time. The TMC2209 needs a minimum
 *  of 100 ns; trying to hit that with a handful of NOPs is unreliable --
 *  the exact width depends on CPU clock and compiler output, and a marginal
 *  pulse may latch on one driver and not on another. Spanning a whole tick
 *  removes the timing question entirely and costs nothing.
 */
void IRAM_ATTR onStepTimer() {
  REG_WRITE(GPIO_OUT_W1TC_REG, STEP_MASK);   // end previous tick's pulse

  uint32_t fire = 0;

  uint32_t i = axTilt.inc;
  if (i) {
    axTilt.acc += i;
    if (axTilt.acc >= 65536UL) { axTilt.acc -= 65536UL; fire |= (1UL << TILT_STEP); }
  }

  i = axPan.inc;
  if (i) {
    axPan.acc += i;
    if (axPan.acc >= 65536UL) { axPan.acc -= 65536UL; fire |= (1UL << PAN_STEP); }
  }

  if (fire) REG_WRITE(GPIO_OUT_W1TS_REG, fire);
}

// Called from loop() every few ms. Ramps rate toward target and, when the
// axis has come to a full stop, applies any pending direction change.
void rampAxis(Axis &a, uint32_t dtMs) {
  int32_t target = (a.cmdDir == 0) ? 0 : a.maxRate;

  // Never reverse while moving -- decelerate to zero first.
  if (a.cmdDir != 0 && a.cmdDir != a.curDir) target = 0;

  int32_t delta = (int32_t)(((int64_t)a.accel * dtMs) / 1000);
  if (delta < 1) delta = 1;

  if      (a.rate < target) { a.rate += delta; if (a.rate > target) a.rate = target; }
  else if (a.rate > target) { a.rate -= delta; if (a.rate < target) a.rate = target; }

  if (a.rate == 0 && a.cmdDir != 0 && a.cmdDir != a.curDir) {
    a.curDir = a.cmdDir;
    Serial.print("DIR pin ");
    Serial.print(a.dirPin);
    Serial.print(" -> ");
    Serial.println((int)a.curDir);
  }

  /*  DIR is written on EVERY pass, not just when it changes. If a single
   *  write were ever missed -- bad solder joint, brownout, glitch -- then
   *  curDir and the physical pin would disagree permanently, and both jog
   *  directions would drive the motor the same way with no way to recover.
   *  curDir only changes while stopped, so rewriting mid-move is harmless.
   */
  bool level = (a.curDir > 0);
  if (a.invert) level = !level;
  digitalWrite(a.dirPin, level ? HIGH : LOW);

  a.inc = (uint32_t)(((uint64_t)a.rate << 16) / TICK_HZ);
}

void stopSteppers() {
  axTilt.cmdDir = 0; axTilt.rate = 0; axTilt.inc = 0; axTilt.acc = 0;
  axPan.cmdDir  = 0; axPan.rate  = 0; axPan.inc  = 0; axPan.acc  = 0;
}

// ==================== drive motors ====================
void driveMotor(int drivePin, int idlePin, int value) {
  value = constrain(value, 0, 255);   // negatives clipped -- no reverse
  if (!armed) value = 0;
  ledcWrite(idlePin, 0);
  ledcWrite(drivePin, value);
}

void applySpeeds() {
  driveMotor(M1_DRIVE, M1_IDLE, speed1);
  driveMotor(M2_DRIVE, M2_IDLE, speed2);
}

void stopAll() {
  speed1 = 0;
  speed2 = 0;
  applySpeeds();
  stopSteppers();
}

void disarm() {
  armed = false;
  stopAll();
  digitalWrite(EN_PIN, LOW);    // BTS7960 off  (active high)
  digitalWrite(TMC_EN, HIGH);   // TMC2209 off  (active LOW)
}

// ==================== web page ====================
const char PAGE_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html><html><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
<title>Rover</title>
<style>
  :root{
    --bg:#12151a; --panel:#1b2029; --line:#2c333f;
    --text:#e8ecf2; --dim:#8b95a5;
    --live:#4fd18b; --idle:#8b95a5; --stop:#e5484d;
  }
  *{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
  body{margin:0;padding:18px;background:var(--bg);color:var(--text);
       font:16px/1.4 ui-monospace,SFMono-Regular,Menlo,monospace}
  h1{font-size:13px;letter-spacing:.18em;text-transform:uppercase;
     color:var(--dim);font-weight:500;margin:0 0 18px}
  .status{display:flex;align-items:center;gap:9px;margin-bottom:20px;font-size:14px}
  .dot{width:9px;height:9px;border-radius:50%;background:var(--idle)}
  .dot.on{background:var(--live);box-shadow:0 0 10px var(--live)}
  .card{background:var(--panel);border:1px solid var(--line);
        border-radius:10px;padding:16px;margin-bottom:12px}
  .row{display:flex;justify-content:space-between;align-items:baseline;margin-bottom:12px}
  .name{font-size:13px;letter-spacing:.1em;text-transform:uppercase;color:var(--dim)}
  .val{font-size:26px;font-variant-numeric:tabular-nums}
  .sub{font-size:12px;color:var(--dim);margin-top:10px;text-align:center}
  input[type=range]{width:100%;height:38px;margin:0;background:transparent;
                    -webkit-appearance:none;appearance:none}
  input[type=range]::-webkit-slider-runnable-track{height:5px;border-radius:3px;background:var(--line)}
  input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:34px;height:34px;
    border-radius:50%;background:var(--text);margin-top:-15px;border:none}
  input[type=range]::-moz-range-track{height:5px;border-radius:3px;background:var(--line)}
  input[type=range]::-moz-range-thumb{width:34px;height:34px;border-radius:50%;
    background:var(--text);border:none}
  .ticks{display:flex;justify-content:space-between;font-size:11px;color:var(--dim);margin-top:2px}
  button{width:100%;padding:16px;font:inherit;font-size:14px;letter-spacing:.12em;
         text-transform:uppercase;border-radius:10px;border:1px solid var(--line);
         background:var(--panel);color:var(--text);margin-bottom:10px}
  button:active{transform:translateY(1px)}
  #arm.on{background:var(--live);border-color:var(--live);color:#0d1117}
  #stop{background:var(--stop);border-color:var(--stop);color:#fff;
        padding:24px;font-size:17px;margin-top:6px}
  .link{display:flex;align-items:center;gap:11px;font-size:14px;color:var(--dim);
        padding:14px 2px}
  .link input{width:22px;height:22px;accent-color:var(--live)}
  .pad{display:grid;gap:8px;touch-action:none;
       grid-template-columns:repeat(3,1fr);
       grid-template-areas:". u ." "l c r" ". d .";}
  .pad button{margin:0;height:62px;font-size:19px;padding:0}
  .pad .u{grid-area:u} .pad .d{grid-area:d}
  .pad .l{grid-area:l} .pad .r{grid-area:r}
  .pad .c{grid-area:c;display:flex;align-items:center;justify-content:center;
          font-size:11px;color:var(--dim);letter-spacing:.1em}
  .jog.act{background:var(--live);border-color:var(--live);color:#0d1117}
  @media (prefers-reduced-motion:reduce){*{transition:none!important}}
</style></head><body>

<h1>Rover Drive</h1>
<div class="status"><div class="dot" id="dot"></div><span id="state">Disarmed</span></div>

<div class="card">
  <div class="row"><span class="name">Motor 1</span><span class="val" id="v1">0</span></div>
  <input type="range" id="s1" min="0" max="255" value="0">
  <div class="ticks"><span>Stop</span><span>Full</span></div>
</div>

<div class="card">
  <div class="row"><span class="name">Motor 2</span><span class="val" id="v2">0</span></div>
  <input type="range" id="s2" min="0" max="255" value="0">
  <div class="ticks"><span>Stop</span><span>Full</span></div>
</div>

<label class="link"><input type="checkbox" id="lk"> Link motors together</label>

<div class="card">
  <div class="row"><span class="name">Aim</span><span class="val" id="aim">--</span></div>
  <div class="pad">
    <button class="jog u" data-ax="t" data-d="1">&#9650;</button>
    <button class="jog l" data-ax="p" data-d="-1">&#9664;</button>
    <div class="c">HOLD</div>
    <button class="jog r" data-ax="p" data-d="1">&#9654;</button>
    <button class="jog d" data-ax="t" data-d="-1">&#9660;</button>
  </div>
  <div class="sub">Up / down tilts &middot; left / right pans</div>
</div>

<button id="arm">Arm motors</button>
<button id="stop">Stop</button>

<script>
const $=id=>document.getElementById(id);
let armed=false, busy=false, pending=false;
let jt=0, jp=0;                       // jog: tilt, pan  (-1 / 0 / +1)

function show(){
  $('v1').textContent=$('s1').value;
  $('v2').textContent=$('s2').value;
  $('dot').className='dot'+(armed?' on':'');
  $('state').textContent=armed?'Armed':'Disarmed';
  $('arm').textContent=armed?'Disarm motors':'Arm motors';
  $('arm').className=armed?'on':'';
  const t=jt>0?'Up':jt<0?'Down':'', p=jp>0?'Right':jp<0?'Left':'';
  $('aim').textContent=(t||p)?[t,p].filter(Boolean).join(' + '):'--';
}

async function send(){
  if(busy){ pending=true; return; }   // never drop the newest state
  busy=true;
  try{
    const q=`/set?m1=${$('s1').value}&m2=${$('s2').value}`
           +`&arm=${armed?1:0}&t=${jt}&p=${jp}`;
    const r=await fetch(q);
    const d=await r.json();
    armed=d.armed;
    if(!armed){ $('s1').value=0; $('s2').value=0; jt=0; jp=0; clearJog(); }
    show();
  }catch(e){ /* keep trying; the ESP32 watchdog stops everything */ }
  busy=false;
  if(pending){ pending=false; send(); }
}

function clearJog(){
  document.querySelectorAll('.jog').forEach(b=>b.classList.remove('act'));
}

document.querySelectorAll('.jog').forEach(btn=>{
  const ax=btn.dataset.ax, d=parseInt(btn.dataset.d,10);
  const press=e=>{ e.preventDefault();
    if(ax==='t') jt=d; else jp=d;
    btn.classList.add('act'); show(); send(); };
  const release=e=>{ e.preventDefault();
    if(ax==='t') jt=0; else jp=0;
    btn.classList.remove('act'); show(); send(); };
  btn.addEventListener('pointerdown',press);
  btn.addEventListener('pointerup',release);
  btn.addEventListener('pointercancel',release);
  btn.addEventListener('pointerleave',release);
  btn.addEventListener('contextmenu',e=>e.preventDefault());
});

function slid(e){
  if($('lk').checked){
    const v=e.target.value;
    $('s1').value=v; $('s2').value=v;
  }
  show(); send();
}
$('s1').addEventListener('input',slid);
$('s2').addEventListener('input',slid);

$('lk').addEventListener('change',()=>{
  if($('lk').checked){ $('s2').value=$('s1').value; show(); send(); }
});

$('arm').addEventListener('click',()=>{
  armed=!armed;
  if(!armed){ $('s1').value=0; $('s2').value=0; jt=0; jp=0; clearJog(); }
  show(); send();
});

$('stop').addEventListener('click',()=>{
  armed=false;
  $('s1').value=0; $('s2').value=0; jt=0; jp=0; clearJog();
  show(); send();
});

// releasing a finger outside the button still has to stop the axis
window.addEventListener('blur',()=>{ jt=0; jp=0; clearJog(); show(); send(); });

setInterval(send,400);   // heartbeat: keeps the ESP32 watchdog fed
show();
</script></body></html>
)HTML";

// ==================== handlers ====================
void handleRoot() {
  server.send_P(200, "text/html", PAGE_HTML);
}

void handleSet() {
  if (server.hasArg("arm")) {
    bool want = server.arg("arm").toInt() == 1;
    if (want != armed) {
      armed = want;
      digitalWrite(EN_PIN,  armed ? HIGH : LOW);    // BTS7960: active high
      digitalWrite(TMC_EN,  armed ? LOW  : HIGH);   // TMC2209: active LOW
      if (!armed) stopAll();
    }
  }

  // constrain() means a hand-typed URL cannot command reverse either
  if (server.hasArg("m1")) speed1 = constrain(server.arg("m1").toInt(), 0, 255);
  if (server.hasArg("m2")) speed2 = constrain(server.arg("m2").toInt(), 0, 255);

  if (server.hasArg("t")) axTilt.cmdDir = constrain(server.arg("t").toInt(), -1, 1);
  if (server.hasArg("p")) axPan.cmdDir  = constrain(server.arg("p").toInt(), -1, 1);
  if (!armed) { axTilt.cmdDir = 0; axPan.cmdDir = 0; }

  applySpeeds();
  lastCommand = millis();

  String json = "{\"armed\":";
  json += armed ? "true" : "false";
  json += ",\"m1\":" + String(speed1);
  json += ",\"m2\":" + String(speed2);
  json += ",\"tilt\":" + String(axTilt.rate);
  json += ",\"pan\":"  + String(axPan.rate) + "}";
  server.send(200, "application/json", json);
}

// ==================== setup ====================
void setup() {
  Serial.begin(115200);

  // safe state FIRST, before anything can move
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);          // BTS7960 disabled
  pinMode(TMC_EN, OUTPUT);
  digitalWrite(TMC_EN, HIGH);         // TMC2209 disabled (active LOW)

  pinMode(TILT_STEP, OUTPUT); digitalWrite(TILT_STEP, LOW);
  pinMode(PAN_STEP,  OUTPUT); digitalWrite(PAN_STEP,  LOW);

  // DIR must match each axis's initial curDir (+1), or the first move in the
  // + direction goes the wrong way: the code thinks no DIR write is needed.
  pinMode(TILT_DIR, OUTPUT); digitalWrite(TILT_DIR, TILT_INVERT ? LOW : HIGH);
  pinMode(PAN_DIR,  OUTPUT); digitalWrite(PAN_DIR,  PAN_INVERT  ? LOW : HIGH);

  // ---- drive PWM ---- (arduino-esp32 v3.x)
  ledcAttach(M1_RPWM, PWM_FREQ, PWM_BITS);
  ledcAttach(M1_LPWM, PWM_FREQ, PWM_BITS);
  ledcAttach(M2_RPWM, PWM_FREQ, PWM_BITS);
  ledcAttach(M2_LPWM, PWM_FREQ, PWM_BITS);
  //
  // v2.x instead:
  //   ledcSetup(0, PWM_FREQ, PWM_BITS); ledcAttachPin(M1_RPWM, 0);
  //   ledcSetup(1, PWM_FREQ, PWM_BITS); ledcAttachPin(M1_LPWM, 1);
  //   ledcSetup(2, PWM_FREQ, PWM_BITS); ledcAttachPin(M2_RPWM, 2);
  //   ledcSetup(3, PWM_FREQ, PWM_BITS); ledcAttachPin(M2_LPWM, 3);
  //   ...and change ledcWrite(pin, v) to ledcWrite(channel, v) in driveMotor().

  // ---- step timer ---- (arduino-esp32 v3.x)
  stepTimer = timerBegin(1000000);                       // 1 MHz time base
  timerAttachInterrupt(stepTimer, &onStepTimer);
  timerAlarm(stepTimer, 1000000 / TICK_HZ, true, 0);     // every 50 us
  //
  // v2.x instead:
  //   stepTimer = timerBegin(0, 80, true);              // 80 MHz / 80 = 1 MHz
  //   timerAttachInterrupt(stepTimer, &onStepTimer, true);
  //   timerAlarmWrite(stepTimer, 1000000 / TICK_HZ, true);
  //   timerAlarmEnable(stepTimer);

  stopAll();

  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("AP up. Connect to \"");
  Serial.print(AP_SSID);
  Serial.print("\" then browse to http://");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.begin();

  lastRamp = millis();
}

// ==================== loop ====================
void loop() {
  server.handleClient();

  // ramp stepper speeds -- every 5 ms is smooth enough and cheap
  unsigned long now = millis();
  if (now - lastRamp >= 5) {
    uint32_t dt = now - lastRamp;
    if (dt > 100) dt = 100;          // web server stalled; don't jump the ramp
    lastRamp = now;
    rampAxis(axTilt, dt);
    rampAxis(axPan,  dt);
  }

  // watchdog: no contact from the phone -> stop
  if (armed && now - lastCommand > TIMEOUT_MS) {
    Serial.println("Timeout - disarming");
    disarm();
  }
}
