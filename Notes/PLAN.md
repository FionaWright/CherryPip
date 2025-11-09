# TODO:

- [x] Run Program  
- [x] Run DX12/WinAPI
- [x] Render RGB triangle
- [x] Clean up code and encapsulate into HWI
- [x] Add frame buffering
- [x] Render spinning cube (Multiple Apps)
- [x] Implement better console/logging
- [x] Implement Dear ImGui
- [x] GLTF Model Importing (fastgltf)
- [ ] Build shaders in cmake
- [x] Hot reloading
- [ ] Add destructors and scene switching
- [x] Clean up CMAKE
- [x] Basic Ray-Tracer (Ray-Query)
- [x] Furnace Test
- [x] Accumulation Buffer
- [x] PT Debug Views & Systems
- [x] NaN Test
- [ ] Switch direction of Engine->App, seperate CMAKE targets
- [ ] Scene, SceneViewer, Refactor App system
- [ ] Save/Load Camera Transform
- [ ] Scene Transforms GUI 
- [ ] CI/CD-like tests
- [ ] Specular/Reflections
- [ ] NEE 
- [ ] Glass & Caustics
- [ ] Ray pipelines

https://learn.microsoft.com/en-us/samples/microsoft/directx-graphics-samples/d3d12-raytracing-samples-win32/

## Optimizations-TODO:

- Frustum Culling
- Texture/Model reuse
- Descriptor heap sharing (Wait to see if it's worth it)
- PCH Compilation? 
- Consider dropping tangent/bitangent from attribute data (Needs CI test to see if it's faster)

## PT-TODO:

### The Big Debug Feature Expansion

- [x] Readback pixel value
- [x] Output Buffers
- [x] Furnace Test
- [x] Change PathTracer to render into a local buffer which debug can modify/use/etc which then gets copied/rendered to the back buffer
- [x] Plot readback data into histogram (I think I should write to a file -> Execute python script -> Use Bokeh)
- [x] Fix whatever makes the furnace test fail
- [ ] RMSE Convergence Graph 

## Notes

Look into russian roulette 

## Scene Refactor part 3

Currently the direction of my engine goes:

main.cpp creates an App vector and passes it into Win32App
Win32App creates an Engine
Engine calls the App functions

What I want: 

main.cpp (within app) passes the single App class into Win32App
client becomes a new target which the app is dependant on
The app has its own CMakeLists.txt

Leave the functionality for multiple Apps running at once jic