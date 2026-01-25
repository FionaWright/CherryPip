# CherryPip

by Fiona Wright

## Info

D3D12 Graphics Engine with a focus on Path-Tracing.  
Built for fun and to further my skills in the graphics programming field.  
Trying to work on it when I have free time from college.  

## Features

### Engine
- Better project structure, engine split between HWI, Core Engine and Apps
- Uses CMAKE for an improved build system
- Dear ImGui for the GUI
- Easy to use readback buffer system for debugging and analysis 
- Python Execution at runtime for data analysis on GPU frame data
- Hot reloading for shaders
- Can render GLTF scenes in both backends
- Set up scene configs and switch between them at runtime seamlessly
- Scene viewer GUI tab where you can check each objects transform, material, etc

### Forward + Deferred Render Backends
- Microfacet lighting model + Irradiance IBL
- Many different debug view modes (WorldPos, Normals, Tangents, Roughness, UV, Lambert, Albedo, etc) (24+)
- Rotatable cubemap skybox that has parity with path tracer backend

### Path Tracer Render Backend
- Using RayQuery through a pixel shader
- Readback debug system that allows you to find selected pixel value or collect data every frame
- Two furnace tests
- Russian Roulette, Firefly Threshold, Importance Sampling
- See different internal parts of the path tracer using Debug Buffer system (Normals, Albedo, HitDist, RNG, FirstBounceDirection, etc) (29+)
- Lambertian, Glossy, Glass and Microfacet lighting models
- GGX and Beckmann NDFs, Schlick Fresnel, Smith Geometry Masking
- Environment maps with support for Panoramic and Octohedral Equal-Area (Rotatable at runtime!)
- Directional lighting + GPU max parallel search on the EA Environment Map to set automatically direction to where luminance is highest
- Denoising (Box, Gaussian, Median, Edge-Avoiding A-Trous) using deferred GBuffer pre-pass

### Spectral Tracer Render Backend
- Work in Progress! 

## History 

Progress as of 29/10/25:
<img width="1699" height="606" alt="image" src="https://github.com/user-attachments/assets/3c638d43-bdc2-465f-8689-2469e6ab254d" />

Progress as of 08/11/25:
<img width="1699" height="599" alt="image" src="https://github.com/user-attachments/assets/8a37f88e-0d83-475e-9b28-3272d3ca2607" />

Progress as of 09/11/25:
<img width="1699" height="604" alt="image" src="https://github.com/user-attachments/assets/ac64c41a-92c5-4159-b324-cfad17abffa0" />

Progress as of 10/01/26 (Took a long break):
<img width="1020" height="573" alt="image" src="https://github.com/user-attachments/assets/f13adf28-8360-4225-a2ab-e718a83e969b" />

Progress as of 18/01/26:
<img width="1696" height="603" alt="image" src="https://github.com/user-attachments/assets/6ae8b25d-a776-4016-bfc3-878b9b3a98fd" />

Progress as of 25/01/26:
<img width="1361" height="578" alt="image" src="https://github.com/user-attachments/assets/60ed04b8-04b0-4655-878b-7c0c79932d9f" />

















