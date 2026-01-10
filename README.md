# CherryPip

by Fiona Wright

## Info

My new DX12 engine with both path-tracing and raster backends. My focus is on path-tracing this time  
Some code was taken from my previous DX12 engine Alkali Engine but most was redesigned due to me being old and wise now   
Trying to work on it during my college semesters in my free time  

## Features

### Engine
- Better project structure, engine split between HWI, Core Engine and Apps
- Uses CMAKE for an improved build system
- Easy to use readback buffer system for debugging and analysis 
- Python Execution at runtime for data analysis
- Hot reloading for shaders
- Can render GLTF scenes in either the rasterizer or path-tracer
- Dear ImGui for the GUI

### Path Tracer
- Using RayQuery through a pixel shader
- See a specific pixels value using readback system
- Two furnace tests
- See different parts of the path tracer using Debug Buffer system (Normals, Albedo, HitDist, RNG, FirstBounceDirection, etc)

## Progress 

Progress as of 29/10/25:
<img width="1699" height="606" alt="image" src="https://github.com/user-attachments/assets/3c638d43-bdc2-465f-8689-2469e6ab254d" />

Progress as of 08/11/25:
<img width="1699" height="599" alt="image" src="https://github.com/user-attachments/assets/8a37f88e-0d83-475e-9b28-3272d3ca2607" />

Progress as of 09/11/25:
<img width="1699" height="604" alt="image" src="https://github.com/user-attachments/assets/ac64c41a-92c5-4159-b324-cfad17abffa0" />

Progress as of 10/01/26 (Took a long break):
<img width="1020" height="573" alt="image" src="https://github.com/user-attachments/assets/f13adf28-8360-4225-a2ab-e718a83e969b" />





