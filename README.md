# Pickle-Ball-Launcher
A pickleball launcher with horizontal and vertical movement, varying distance, and varying spin.\
Check out my video series here: https://www.youtube.com/@lchi_builds

# Challenges
The motor shafts used to rotate the scooter wheels were only 16mm in length each. This meant if I made a full front mount utilizing the screw holes in the front on the motor, the pickleball wouldn't fit.\

Segment gear teeth not concentric with the pivot axis:\
The driving spur pinion could not move the segment gear through its full travel. Mesh was correct at one position, but binding or backlash grew as the segment rocked away from that point, and eventually the teeth stopped engaging altogether.\
The fix: I rebuilt the segment gear so that its teeth and its pivot share a single axis. Instead of cutting teeth about one center and the pivot bore at another, I generated a complete gear centered on the pivot axis, then removed the unused material from that full gear. The remaining teeth all lie on one pitch circle concentric with the pivot, so the center distance to the pinion stays constant through the entire sweep.

https://github.com/user-attachments/assets/a06b840f-46ca-4a9e-8af4-df5ab2cb8a56



