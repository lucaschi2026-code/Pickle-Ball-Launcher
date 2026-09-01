# Pickle-Ball-Launcher
A pickleball launcher with horizontal and vertical movement, varying distance, and varying spin. Project was unsuccessful. Read more below.

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

Things that didn't work, why, and what I'd change.

---

## 1. Tilt axis wouldn't come back down

**What happened**

- Pinion turned the segment gear, but the mount only moved one way — tilt up and it stayed up, commanding down did nothing.
- Superglueing the lower motor to the segment gear worked briefly, then motor vibration broke the joint.

**Why**

- The segment gear was never positively fastened to the mount — only glue and friction.
- That makes a one-way drive: going up, the gear pushes the mount along. Coming down, the gear has to *pull* the mount, and the joint couldn't carry the weight of two motors in shear.
- Superglue (cyanoacrylate) is brittle — decent in a straight pull, almost no fatigue life under vibration. Two spinning motors right next to the joint is the worst case for it.
- The tilt assembly wasn't balanced on its pivot, so gravity and pivot friction were always working against the motor.

**Improvements**

- Print the segment gear as part of the tilt mount — no joint left to fail.
- Balance the assembly (move mass or add a counterweight) so the centre of gravity sits on the pivot axis. The motor then only fights friction, not weight.

---

## 2. Base vibrates and goes unstable at high speed

**What happened**

- Fine at low launcher speed. Spin the wheels up and the whole base shakes and walks.

**Why**

- The lazy susan is a printed roller thrust bearing — rollers in a flat channel. It holds weight fine, but nothing holds the top ring *centred*.
- Too much clearance which allows the top ring to shift horizontally in all directions
- Unbalanced spinning launcher wheels produce a force that grows with the *square* of speed — double the RPM, four times the shaking. More than enough to rattle the top ring through all that clearance.
- Printed rollers aren't perfectly round or all the same diameter, so they skew in the channel and chatter instead of rolling.

**Improvements**

- Buy the bearing: a steel lazy-susan turntable bearing or thin-section slewing ring handles both up/down and sideways load properly. This is the real fix.
- If keeping it printed: add the raised hub lip on the bottom ring that engages the top ring's 50 mm bore (already flagged in the design notes). This is the one change that kills the sideways drift.
- Rubber feet or a weighted base so it can't walk across the floor.

---

## Why the pickleball launcher failed

This is the one that actually stopped the project working, so it gets its own section.

### What happened

- At low wheel speed the ball launched, but weakly.
- Turning the speed up made it *worse* — the ball dribbled out of the wheels instead of flying.
- More speed never added range.

### How a wheel launcher is supposed to work

- Two counter-rotating wheels squeeze the ball; friction drags it up to the **surface speed** of the wheels and it exits at about that speed.
- `surface speed = π × wheel diameter × RPM`
- This only holds if the ball doesn't **slip**. More wheel speed giving less ball speed is the signature of a traction problem, not a power problem.

### Why it failed

- **Tiny contact patch.** Scooter tread is narrow and rounded (~24–30 mm wide). A pickleball is a 74 mm sphere. Round pressed on round is close to a single *point*, and friction force depends on how much surface is touching.
- **A pickleball won't squash — this is the core of it.** A tennis ball flattens and wraps around the wheel, giving a big patch and long contact; that's why narrow wheels work in tennis machines. A pickleball is hard, hollow, perforated plastic. Press harder and you just crack it. All the give has to come from the wheel, and scooter wheels have almost none.
- **Contact lasts only a few milliseconds.** Speed comes from force × time. Small force × short time = slow ball. And faster wheels make that window *shorter*.
- **Both surfaces are slippery.** Smooth hard plastic on smooth polyurethane, and the holes in the ball break up what little patch there is.
- **The wheels bogged down on impact.** Light wheels store little rotational energy, and DC motor torque sags under a sudden current spike — so the wheel slows at the exact moment it should be gripping.
- **Why more speed made it worse:** a faster launch needs more force over less time. The friction *needed* goes up, the friction *available* stays the same, so more of the wheel speed is lost to slip.

### The motor probably wasn't the problem

- A pickleball weighs ~24 g. To reach 20 m/s (~72 km/h): `½ × 0.024 × 20² ≈ 5 J`.
- 5 J is nothing. Even delivered in 10 ms that's a ~500 W spike, which a flywheel can supply on its own between shots without the motor ever seeing it.
- The energy was there — the ball just wasn't gripped hard enough or long enough to receive it. Swapping motors alone would not have fixed this.

### Improvements

- **Concave (grooved) wheels.** Cut or print a groove matching the ball radius (~37 mm) so the wheel cradles the ball. Biggest single change: point contact becomes a wide wrapping patch, and the ball can't squirt sideways out of the nip.
- **Brushless outrunners + ESCs on 4S/6S** if more headroom is still needed. This is what commercial ball machines and FRC shooters use — but do it *after* fixing the contact geometry, not instead of it.
- **A guide throat before and after the wheels** so the ball always enters dead centre.

### How to check whether a fix worked

- Film the launch in phone slow-motion and measure exit speed against a marked wall.
- `efficiency = ball speed ÷ wheel surface speed`
- Under ~50 % means it's still slipping badly. A good wheel launcher lands above ~80 %.

---

## 4. No ball feeder

- Not a failure — it just wasn't built this time. Balls were loaded by hand.

**For a future version**

- Gravity hopper feeding a rotating disc **escapement** (a slotted wheel that lets one ball through per turn) or a servo gate.
- A delay between shots so the wheels can spin back up — firing early gives inconsistent range.
- Optional RPM sensor so it fires when the wheels are actually ready, instead of on a fixed timer.

---

## 5. Electronics had nowhere to live

**What happened**

- The circuit ended up strapped to the outside of the frame — messy, bulky, exposed. Wiring ran wherever it could reach.

**Why**

- The enclosure and cable routing were never part of the mechanical design. Mechanics were designed first, electronics second, and by the time the boards existed no space had been reserved.
- Also a safety issue: bare LiPo-level wiring next to moving parts and vibrating motors.

**Improvements**

- Reserve the electronics bay in CAD at the same time as the mechanics — define the box volume and mounting bosses before the frame is finalised.
- Custom PCB: the BTS7960, TMC2209 and ESP32 modules become sockets on one board. No jumper wires, far smaller, far more reliable.



