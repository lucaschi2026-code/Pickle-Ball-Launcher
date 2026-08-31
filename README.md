# Pickle-Ball-Launcher
A pickleball launcher with horizontal and vertical movement, varying distance, and varying spin.\
Check out my video series here: https://www.youtube.com/@lchi_builds

# Challenges
### Short Motor Shaft

**Problem**

- Motor shafts were only 16mm in length each
- Pickleball wouldn't fit if I used a full front mount only using the screw holes in the front of the motor for mounting

**Cause**

- I bought the wrong motor :(

**Fix**

- Utilized a partial front mounts to allow space between the motors to accomadate for the pickleball
- Secured the partial front mounts to a back mount with rods for more stability
<table>
  <tr>
    <td align="center"><img src="https://github.com/user-attachments/assets/27272106-f529-470b-b4ff-c9c546e4dd82" width="200"/><br><sub>Problem</sub></td>
    <td align="center"><img src="https://github.com/user-attachments/assets/c17f47db-585d-467f-87f6-56ed3d80a6e9" width="200"/><br><sub>Solution</sub></td>
  </tr>
</table>


### Segment gear teeth not concentric with the pivot axis

Video explanation: [YT short](https://youtube.com/shorts/Wz2A-TTkKP4?si=VPnpqFrX6oq3ApiZ)

**Problem**

- The spur pinion could not drive the segment gear through its full travel.
- Mesh was correct at only one position in the sweep.
- Moving away from that position, the pair progressively bound up in one direction and gained backlash in the other.
- At the extremes, the teeth stopped engaging altogether.

**Cause**

- The teeth had been generated about the donor gear's center, while the part rotates about its pivot bore — two different axes.
- That offset makes the pitch circle swing rather than spin, so the pinion-to-segment center distance varies with sweep angle.

**Fix**

- Rebuilt the segment gear with its teeth and pivot on a single axis.
- Generated a complete gear centered on the pivot axis, then removed the unused material.
- All remaining teeth now lie on one pitch circle concentric with the pivot, so center distance to the pinion stays constant across the entire sweep.

https://github.com/user-attachments/assets/aec71726-0927-466b-a4a0-04b81f9691da

https://github.com/user-attachments/assets/82309e48-c833-469f-9269-4ec8329cf462

### Motor Arm Enclosure Walls Too High

**Problem**
- Stepper motor would not seat into the motor arm enclosure
- The walls on the back side of the enclosure were too high and blocked the motor from dropping in

**Cause**
- The enclosure was modeled around NEMA 17 stepper motor, but I did not consider how I was going to get the motor in the enclosure in the first place

**Fix**
- Melted the back walls off with a soldering iron and removed them
- Fixed 3D model
- Motor drops in cleanly now — the four M3 bolts through the front wall plus the rest pads hold it securely, so the back walls turned out to be unnecessary

<table>
  <tr>
    <td align="center"><img src="https://github.com/user-attachments/assets/f936857b-2f46-4ab9-b7a9-50f561dcf009" width="200"/><br><sub>Back View</sub></td>
    <td align="center"><img src="https://github.com/user-attachments/assets/1d6f63dc-018e-478e-8d47-2537d4a96baa" width="200"/><br><sub>Front View</sub></td>
    <td align="center"><img src="https://github.com/user-attachments/assets/2ee49d61-6207-4e40-b8a6-192d4ce8fe5e" width="200"/><br><sub>Side View</sub></td>
    <td align="center"><img src="https://github.com/user-attachments/assets/bc4cdb7b-24c7-493a-84d5-169191941115" width="200"/><br><sub>Solution</sub></td>
  </tr>
</table>

# Improvements & Lessons Learned

Things that didn't work, why they didn't work, and what I'd change.

---

## 1. Tilt axis wouldn't come back down

**What happened**

The pinion turned the segment gear, but the launcher mount only really moved one way. Tilt it up and it stayed up; commanding it back down did nothing. Superglueing the lower motor to the segment gear worked for a little while, then motor vibration broke the joint.

**Why**

The segment gear was never *positively fastened* to the tilt mount — it was held by glue and friction. That is a one-way drive:

- Going up, the gear pushes into the mount, so the mount comes along.
- Coming down, the gear has to actually **pull** the mount. Glue in shear plus the weight of two motors was more than the joint could carry, so the gear just moved and the mount stayed put.

Two things made it worse:

- **Superglue (cyanoacrylate) is brittle.** It's strong in a straight pull but has almost no fatigue life under vibration. Two spinning motors bolted right next to the joint is the worst possible load for it.
- **The tilt assembly wasn't balanced on its pivot.** Its centre of mass sat off the pivot axis, so gravity and pivot friction were always working against the motor instead of cancelling out.

**Improvements**

- **Bolt the gear on, don't glue it.** Three or four M3 screws on a circle around the pivot bore, plus a locating pin. Use nyloc nuts or threadlocker — plain nuts back off under vibration.
- **Better: make it one part.** Print the segment gear as part of the tilt mount so there's no joint to fail at all.
- **Put a real bearing at the pivot** (608 bearing or a flanged bushing) instead of plastic on plastic, and don't overtighten the pivot bolt. Less friction to fight.
- **Balance the tilt assembly.** Move mass or add a counterweight so the centre of gravity sits on the pivot axis. Then the motor only has to overcome friction, not lift the whole head.
- **If it still can't hold position, use a worm drive on tilt too.** A worm is self-locking (it can't be back-driven), so the axis holds where you leave it with no power.

---

## 2. Base vibrates and goes unstable at high speed

**What happened**

At low launcher speeds the base is fine. Spin the wheels up and the whole thing starts shaking and walking.

**Why**

The lazy susan is a printed **roller thrust bearing** — cylindrical rollers running in a flat channel. It holds weight fine, but nothing holds the top ring *centred*. From the design notes there's 0.7 mm vertical clearance, 1.5 mm end play, and **no radial location at all** (nothing stops the rings sliding sideways relative to each other).

That slop on its own is harmless. The problem is what sits on top of it: spinning launcher wheels that aren't perfectly balanced. An out-of-balance wheel makes a force that goes around once per revolution, and that force grows with the *square* of speed. Double the RPM, four times the shaking. At high speed it's more than enough to rattle the top ring around inside all that clearance.

Printed rollers make it worse — they're not perfectly round and not all exactly the same diameter, so they skew in the channel and chatter instead of rolling smoothly. The keeper shoes limit sideways drift to ~0.4 mm, but that's a patch, not a fix.

**Improvements**

- **Buy the bearing.** A steel lazy-susan turntable bearing or a thin-section slewing ring is cheap and handles both up/down and sideways load properly. This is the real fix.
- **If keeping it printed:** add the raised hub lip on the bottom ring that engages the top ring's 50 mm bore. This was already flagged in the design notes as the proper fix and it's the one change that kills the sideways drift.
- **Preload out the vertical gap.** Shim the channel so the rollers are lightly squeezed rather than rattling in 0.7 mm of air.
- **Use steel rolling elements** — 6 mm steel balls or cut dowel pins — instead of printed rollers. Round, hard, identical.
- **Balance the launcher wheels.** Spin them up and add small weights (tape works) until the shaking stops. This alone will remove most of the input energy.
- **Lower the centre of gravity** and widen the base footprint. A tall, top-heavy assembly turns a small wobble into a big one.
- **Rubber feet or a weighted base** so it can't walk across the floor.

---

## Why the pickleball launcher failed

This is the one that actually stopped the project working, so it gets its own section.

### What happened

At low wheel speed the ball would launch, but weakly. Turning the speed up made it *worse* — the ball dribbled out of the wheels slowly instead of flying. Adding more speed did not add more range.

### How a wheel launcher is supposed to work

Two counter-rotating wheels squeeze the ball. Friction between the tread and the ball drags it up to roughly the **surface speed** of the wheels, and it exits at about that speed.

```
surface speed = π × wheel diameter × RPM
```

That only holds if the ball doesn't **slip**. Any slip is speed you paid for and didn't get. The symptom — more wheel speed, less ball speed — is the signature of a traction problem, not a power problem.

### Why it failed

**1. The contact patch was tiny.**
Scooter wheels have a narrow, rounded (crowned) tread, roughly 24–30 mm wide. A pickleball is a 74 mm sphere. Round pressed against round gives you close to a *point* of contact. Friction force scales with how hard you press and how much surface is touching — and there was barely any surface touching.

**2. A pickleball won't squash.**
This is the core of it. A tennis ball is soft: it flattens against the wheel, wraps around it, and gives you a big contact patch and a long contact time — that's why tennis ball machines work with narrow wheels. A pickleball is hard, hollow, perforated plastic with almost no give. You can't squeeze your way to a bigger patch; press harder and you just crack the ball. **All the compliance has to come from the wheel, and scooter wheels have almost none.**

**3. The ball is only in the nip for a few milliseconds.**
Speed comes from force × time (impulse). Small force from a point contact, multiplied by a very short contact time, equals a slow ball. And the faster the wheels spin, the *shorter* that contact time gets — so the window shrinks exactly when you need more from it.

**4. Both surfaces are slippery.**
Smooth hard plastic against smooth polyurethane tread. The holes in the ball break up the patch even further — some of the tiny contact area is literally hole.

**5. The wheels bogged down on impact.**
When the ball hits, the wheel takes a sudden load spike. Scooter wheels are fairly light, so there isn't much stored rotational energy (flywheel effect) to ride it out, and DC motor torque sags under a sudden current draw. So the wheel slows down at the exact moment it should be gripping.

**Why more speed made it worse, specifically:** to launch faster, the wheels have to accelerate the ball harder over a shorter contact time. The friction *needed* goes up; the friction *available* stays the same. So the fraction of wheel speed lost to slip goes up. Past a certain point you're just polishing the ball.

### The motor probably wasn't the problem

Worth doing the maths, because it changes what you'd fix. A pickleball weighs about 24 g. Getting it to 20 m/s (~72 km/h) takes:

```
½ × 0.024 kg × 20² = ~5 joules
```

Five joules is nothing. Even delivered in 10 ms that's a ~500 W spike, and a flywheel with enough mass can supply that spike on its own between shots without the motor ever seeing it. The motors were not short of energy — **the ball just wasn't gripped hard enough or long enough to receive it.** Swapping motors alone would not have fixed this.

### Improvements

Roughly in order of how much they'd help:

- **Concave (grooved) wheels.** Cut or print a curved groove into the tread matching the ball radius (~37 mm) so the wheel cradles the ball instead of poking it. This is the single biggest change — it turns a point contact into a wide wrapping patch, and it stops the ball squirting sideways out of the nip.
- **Softer, wider tread.** A printed hub with a TPU tire (roughly 40–60 Shore A) or a foam wheel. Since the ball can't deform, the wheel has to.
- **Spring-loaded gap instead of a fixed gap.** Mount one wheel on a pivot with a spring so it presses with a set *force*. You get consistent squeeze, and it tolerates ball-to-ball size variation and tread wear.
- **More wrap.** Add a curved guide channel that keeps the ball pressed against the wheel over more of its circumference, or go to a 3- or 4-wheel arrangement, to buy more contact time.
- **Heavier flywheel.** Add mass to the wheels (or a separate flywheel disc on the shaft) so the speed doesn't collapse on each shot. Let it spin back up between balls.
- **More headroom on power, if still needed:** brushless outrunners + ESCs on 4S/6S. This is what commercial ball machines and FRC shooters use. Do this *after* fixing the contact geometry, not instead of it.
- **A guide throat before and after the wheels** so the ball always enters dead centre.

### How to check if a fix worked

Film the launch on a phone in slow motion and measure the ball's exit speed against a ruler or marked wall. Compare it to the wheel surface speed calculated from RPM:

```
efficiency = ball speed ÷ wheel surface speed
```

Anything under ~50 % means you're still slipping badly. A good wheel launcher should land above ~80 %.

---

## 4. No ball feeder

Not a failure — it just wasn't built this time. Balls were loaded by hand.

**For a future version**

- Gravity hopper feeding a rotating disc **escapement** (a slotted wheel that only lets one ball through per turn) or a servo gate.
- One ball at a time, with a delay between shots so the wheels can spin back up to speed. Firing before the wheels recover gives inconsistent range.
- Optional: an RPM sensor so it fires when the wheels are actually ready, instead of on a fixed timer.

---

## 5. Electronics had nowhere to live

**What happened**

The circuit was built on perfboard and ended up strapped to the outside of the frame — messy, bulky, and exposed. Wiring ran wherever it could reach.

**Why**

The enclosure and the cable routing were never part of the mechanical design. The mechanics were designed first, the electronics were designed second, and by the time the boards existed there was no space reserved for them. It's also a safety issue: bare LiPo-level wiring sitting next to moving parts and vibrating motors.

**Improvements**

- **Reserve the electronics bay in CAD at the same time as the mechanics.** Define a box volume with mounting bosses before the frame is finalised, not after.
- **Move from perfboard to a custom PCB.** The BTS7960, TMC2209 and ESP32 modules all become sockets on one board — no jumper wires, far smaller, far more reliable.
- **Printed enclosure with a lid**, cable glands or grommets where wires pass through, and cut-outs for the USB port, main switch, and fuse holders so you don't have to open it to reset a fuse or reflash.
- **Use connectors, not soldered wire runs.** XT60 for battery, JST-XH for logic, screw terminals with crimped ferrules for motors. Serviceable, and much better under vibration.
- **Strain relief on every bundle**, and keep motor wires physically separated from logic wires (motor wires are electrically noisy).
- **Airflow** — a slot or small fan over the motor drivers, which are the hottest parts.

---

## Common thread

Two themes run through almost all of this:

1. **Vibration destroys anything that isn't mechanically fastened.** It broke the tilt gear glue joint, it exposed every bit of slop in the lazy susan, and it's the main long-term threat to the exposed wiring. Bolts and positive locating features, not adhesive and friction.
2. **Grip beats power.** The launcher didn't fail for lack of motor — it failed because the contact between wheel and ball was too small, too brief, and too slippery. The next version starts with the wheel profile, not the motor spec.



