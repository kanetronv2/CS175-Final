CS175: PSET 4
Konlin Shen
10/18/11

List of Files Submitted:
asst4.cpp
cvec.h
drawer.h
geometry.h
glsupport.cpp
glsupport.h
matrix4.h
ppm.cpp
ppm.h
quat.h
arcball.h
rigtform.h
scenegraph.cpp
scenegraph.h
picker.cpp
picker.h
asstcommon.h
basic.vshader
basic-gl3.vshader
diffuse.fshader
diffuse-gl3.fshader
solid.fshader
solid-gl3.fshader
pick-gl3.fshader
README.txt

Development Platform:
Windows 7

How to Compile/run:
Open up files in Microsoft Visual Studio then build and run (press f5)!

Problem Set Requirements:
I believe I have met all the problem set requirements.  I have written the proper visitor and picker classes, allowed the user to choose what object
to manipulate by pressing 'p' and clicking, established a hierarchy in the robot manipulation, established robot manipulation, and gave the robots
a head/arms/legs/lower legs. There is a minor bug, however, in that it seems like when you try to manipulate the other robots from the view of a
different robot, the arcball kind of messes up the movement.  Sometimes this doesn't happen though, so I don't know if it's just messy mouse-movement
or an actual bug.

Code Design:
Really not much to say here.  Changed everything into nodes as per spec, then if we are moving a robot piece, we move it via the auxiliary frame
formula given in the spec.  Ego-motion has no arcball, of course, so we had to set ego-motion apart from the rest of motion, but other than that 
everything is about the same as listed in the spec.  

How to Run the Program:
Press 'p' to go into picking mode, then click to select an object.  Again, right-clicking and moving will rotate, left-clicking and moving will
translate, and middle-clicking and moving will translate as well.  Pressing 'v' will still change the view and pressing 'm' will toggle ego-mode
if you are viewing from the sky-camera.    

Above and Beyond:
Not really.  