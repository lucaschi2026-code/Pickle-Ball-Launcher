# Pickle-Ball-Launcher
A pickleball launcher with horizontal and vertical movement, varying distance, and varying spin.\
Check out my video series here: https://www.youtube.com/@lchi_builds

# Challenges
The motor shafts used to rotate the scooter wheels were only 16mm in length each. This meant if I made a full front mount utilizing the screw holes in the front on the motor, the pickleball wouldn't fit.
&nbsp;
### Segment gear teeth not concentric with the pivot axis

Video explanation:

&nbsp;
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






