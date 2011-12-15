Kane Hsieh
CS175 Fall 2011
Assignment 10

/*
 * List of all files you submitted.
 */

asst10.cpp
asst10.sln
asst10.vcxproj
asst10.vcxproj.filters
cvec.h
final.ppm
Makefile
output_image.ppm
ppm.cpp
ppm.h
raycast.cpp
raycast.cpp~
raycast.h
ray.h
#scene.cpp#
scene.cpp
scene.h
scene.txt
surface.h
task1.ppm
task2.ppm

/*
 * Note the platform you used for development
 */

Science Center Linux, borrowed Windows 7, borrowed Ubuntu 10.10


/*
 * Provide instructions on how to compile and run your code
 */

Provided Makefile works


/*
 * Indicate if you met all problem set requirements
 */

Yup.


/*
 * Provide some overview of the code design. Don't go into details; just give us the big picture.
 */

I followed the spec pretty straightforward. Used the formals in Chapter 20 of the book to implement intersect() and computeNormal() in surface.h, and getBounce() in raycast.cpp.

Shadows used a "shadow" ray, and if the lambda indicated that an intersection occured, no color was returned (thus shadow). For mirror, I used the Cvec functions to linearly blend the colors as determined by rayTrace() if the mirrorCoef was positive (indicating that the surface intersected was a mirrored surface).


/*
 * Let us know how to run the program
 */

Provided spec works


/*
 * Finally, did you implement anything above and beyond the problem set?
 */

Nope.
