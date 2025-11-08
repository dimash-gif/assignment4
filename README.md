
# OpenGL Graphics Demonstrations - Assignment 4

## Overview
This repository contains four C++ programs demonstrating fundamental concepts in real-time 3D computer graphics using OpenGL. The projects explore parametric surfaces, object picking with framebuffers, and different methods of texture mapping. Each program is a standalone interactive demo.

To build the project:
```bash 
make
```

## Interactive Bézier Patch (assignment4_part1)

This program renders a 3D Bézier patch from a 4x4 grid of control points. You can interactively select and move the control points to see the surface update in real-time.

To run the code:

```bash
./assignment4_part1
```

#### Controls
| Key / Action | Description |
|---------------|-------------|
| **Left Mouse Click** | Pick an object to change its color |
| **W / S** | Adjust Camera Pitch (Up / Down) |
| **A / D** | Adjust Camera Angle (Orbit Left / Right) |
| **Z / X** | Zoom Camera In / Out |
| **0 | - 9** | Directly select control points 0 through 9 |
| **U | J** | Move the selected control point along the X-axis |
| **I | K** | Move the selected control point along the Y-axis |
| **O | L** | Move the selected control point along the Z-axis |
| **ESC** | Exit the program |

## Interactive Picking (assignment4_part2)
To run the code:
```bash
./assignment4_part2
```
#### Controls
| Key / Action | Description |
|---------------|-------------|
| **Left Mouse Click** | Pick an object to change its color |
| **W / S** | Adjust Camera Pitch (Up / Down) |
| **D / Q** | Adjust Camera Angle (Orbit Left / Right) |
| **Z / X** | Zoom Camera In / Out |
| **R** | Reset the camera to the default view |
| **A** | Toggle MSAA Anti-aliasing On / Off |
| **ESC** | Exit the program |

## Bézier Patch with Procedural Texture (texture_mapping)



To run the code:

```bash
./texture_mapping
```
#### Controls
| Key / Action | Description |
|---------------|-------------|
| **W / S** | Adjust Camera Pitch (Up / Down) |
| **A / D** | Adjust Camera Angle (Orbit Left / Right) |
| **Z / X** | Zoom Camera In / Out |
| **ESC** | Exit the program |

## 3D Procedural Wood Texture (shading_demo)

To run the code:

```bash
./shading_demo models/bound-lo-sphere.smf
```
#### Controls
| Key / Action | Description |
|---------------|-------------|
| **Left Mouse Click** | Pick an object to change its color |
| **W / S** | Zoom Camera In / Out |
| **A / D** | Orbit Camera Left / Right |
| **Q / E** | Raise / Lower Camera Height |
| **P** | Toggle between Perspective and Orthographic projection |
| **<- | ->** | Orbit the Light Source around the model |
| **U / O** | Raise / Lower the Light Source |
| **I / K** | Move the Light Source Closer / Farther |
| **Q / E** | Raise / Lower Camera Height |
| **ESC** | Exit the program |


