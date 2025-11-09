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
- [ ] NaN Test, split Furnace and DebugBuffer
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

## The Scene Refactor

I want to be able to do:
PtContext->Render(bistroScene)
RasterContext->Render(bistroScene)

But ptcontext already holds scene data 

New Idea:
PtContext holds frameCount, accumTex, fullscreenTriangle, etc
Scene holds list of objects, BLAS/TLAS/InstanceDataBuffer/etc, 