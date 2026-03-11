# CherryPip

by Fiona Wright

## Info

D3D12 Graphics Engine with a focus on Path-Tracing using RayQuery.  
Built for fun and to further my skills in the graphics programming field.  
Trying to work on it when I have free time from college.  

Uses CMAKE for the build system.  
C++, HLSL and Python are the main languages.  

### Third-Party Tools
- Dear ImGui (GUI)
- spng (PNG Loader)
- tinyddsloader (DDS Loader)
- texconv (PNG/JPG -> DDS Build Step)
- zlib/fastgltf (GLTF Loader)
- WinPixEventRuntime (GPU Events/Markers) 

## Features

### Engine
- Repository split between engine (HWI + System + Render) and Apps (SceneStudio)
- Python Execution at runtime for data analysis / debugging
- Hot reloading for shaders
- Set up scene configs and switch between them at runtime seamlessly
- Scene GUI tab where you can modify each objects transform, material, etc

### Forward + Deferred Render Backends
- Microfacet lighting model + Irradiance IBL
- Rotatable cubemap skybox that has parity with path tracer backend
- Many different debug view modes (WorldPos, Normals, Tangents, Roughness, UV, Lambert, Albedo, etc) (Over 20)
- Ability to render lines for debugging directions/axes/etc

### Path Tracer Render Backend
- Fully deterministic and seeded
- Russian Roulette, Firefly Threshold, Cosine Importance Sampling
- Furnace Tests for debugging
- Depth of Field
- Independent, Halton, Apple Halton and Owen-Scrambled Halton RNG sampling strategies
- Environment maps with support for Panoramic and Octohedral Equal-Area (Rotatable at runtime)
- Directional lighting + GPU max parallel search on the EA Environment Map to set automatically direction to where luminance is highest
- Denoising (Box, Gaussian, Median, Edge-Avoiding A-Trous) using GBuffer pre-pass

#### Lighting Models
- Lambertian
- Glossy
- Glass
- GGX Smith
- GGX Smith Anisotropic
- GGX VCavity VNDF
- GGX Smith VNDF
- GGX Smith Anisotropic VNDF

#### Debug Tools
##### Readback System
- Click on a pixel and see its RGBA output value in the GUI
- Automatically collect a pixels value over multiple frames and then plot it in a histogram
##### Info Output System
- Output different variables inside the path-tracer for each pixel (Normals, Albedo, HitDist, RNG, FirstBounceDirection, etc) (Over 30)
##### Convergence System
- Take snapshots of the path-tracer output into 2 slots and find the RMSE between them with a compute shader
- Generate golden images, save them as PNGs and load them into slot A
- Automatically take snapshots into slot B every frame and compute the RMSE. Then plot RMSE convergence graphs using python
##### Path Visualizer System
- Click on a pixel to have all rays shot from that pixel next frame store their positions at each bounce
- That data is then converted into coloured lines so you can follow the rays path

### (WIP) Spectral Tracer Render Backend
- Work in progress, currently doing research

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
<img width="1361" height="575" alt="image" src="https://github.com/user-attachments/assets/cf597041-cf49-4f93-9f9e-9f91ccc713d5" />

Progress as of 29/01/26:  
<img width="961" height="575" alt="image" src="https://github.com/user-attachments/assets/dc7b6217-9eaa-4a9b-ad21-7b998f25ba47" />

Progress as of 18/02/26:
<img width="1020" height="575" alt="Chess Beauty" src="https://github.com/user-attachments/assets/e3467b67-f8c4-4d19-aa1f-199c96550b2e" />

Progress as of 21/02/26:
<img width="1020" height="575" alt="Chess DOF" src="https://github.com/user-attachments/assets/a6b0518d-efac-41c1-a811-cda16036083c" />

Progress as of 11/03/26 (Spectral-tracing working! This is after 30 frames):
<img width="1016" height="574" alt="image" src="https://github.com/user-attachments/assets/6e10fb11-948b-412c-bfe2-33774b99ac34" />












