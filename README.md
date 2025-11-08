
# OpenGL Graphics Demonstrations - Assignment 4

## Overview
This repository contains four C++ programs demonstrating fundamental concepts in real-time 3D computer graphics using OpenGL. The projects explore parametric surfaces, object picking with framebuffers, and different methods of texture mapping. Each program is a standalone interactive demo.

## How to build and run the code
Firstly, ensure you have the necessary prerequisites installed on your Linux system: a C++ compiler (`g++`), `make`, and the `GLFW` and `OpenGL` development libraries.

To build the project:
```bash 
make
```
This will compile all four executables.

To run each program, see the specific instructions below.

Programs and Controls
1. Interactive Bézier Patch (assignment4_part1)

This program renders a 3D Bézier patch from a 4x4 grid of control points. You can interactively select and move the control points to see the surface update in real-time.

To run the code:

code
Bash
download
content_copy
expand_less
./assignment4_part1
Camera Controls
Key / Action	Description
W / S	Adjust Camera Pitch (Up / Down)
A / D	Adjust Camera Angle (Orbit Left / Right)
Z / X	Zoom Camera In / Out
R	Reset the camera to the default view
Patch Controls
Key / Action	Description
← / →	Cycle through the 16 control points
0 - 9	Directly select control points 0 through 9
U / J	Move the selected point along the X-axis
I / K	Move the selected point along the Y-axis
O / L	Move the selected point along the Z-axis
+ / -	Increase / Decrease the tessellation level
General
Key	Action
ESC	Exit the program
2. Interactive Picking (assignment4_part2)

This program demonstrates object picking using an off-screen framebuffer (FBO). The scene contains three objects (sphere, cube, cone). Clicking on an object will randomize its color.

To run the code:

code
Bash
download
content_copy
expand_less
./assignment4_part2
General Controls
Key / Action	Description
Left Mouse Click	Pick an object to change its color
A	Toggle MSAA Anti-aliasing On / Off
ESC	Exit the program
Camera Controls
Key / Action	Description
W / S	Adjust Camera Pitch (Up / Down)
D / Q	Adjust Camera Angle (Orbit Left / Right)
Z / X	Zoom Camera In / Out
R	Reset the camera to the default view
3. Bézier Patch with Procedural Texture (texture_mapping)

This program renders a curved Bézier patch and applies a procedural rainbow ring texture with a Phong specular highlight, similar to the requested image.

To run the code:

code
Bash
download
content_copy
expand_less
./texture_mapping
Controls
Key / Action	Description
W / S	Adjust Camera Pitch (Up / Down)
A / D	Adjust Camera Angle (Orbit Left / Right)
Z / X	Zoom Camera In / Out
ESC	Exit the program
4. 3D Procedural Wood Texture (shading_demo)

This program loads a 3D model from an .smf file and applies a 3D procedural wood grain texture. The color is calculated based on the world-space (X, Y, Z) coordinates of each fragment.

To run the code:

code
Bash
download
content_copy
expand_less
./shading_demo models/bound-lo-sphere.smf
Camera Controls
Key / Action	Description
W / S	Zoom Camera In / Out
A / D	Orbit Camera Left / Right
Q / E	Raise / Lower Camera Height
P	Toggle between Perspective and Orthographic projection
Light Controls
Key	Action
← / →	Orbit the Light Source around the model
U / O	Raise / Lower the Light Source
I / K	Move the Light Source Closer / Farther
General
Key	Action
ESC	Exit the program
code
Code
download
content_copy
expand_less
