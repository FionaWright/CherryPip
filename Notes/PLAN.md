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
- [x] Add destructors and scene switching
- [x] Clean up CMAKE
- [x] Basic Ray-Tracer (Ray-Query)
- [x] Furnace Test
- [x] Accumulation Buffer
- [x] PT Debug Views & Systems
- [x] NaN Test
- [x] Switch direction of Engine->App, seperate CMAKE targets
- [x] Scene, SceneViewer, Refactor App system
- [x] Save/Load Camera Transform
- [ ] Scene Transforms/Materials GUI
- [ ] CI/CD-like tests
- [x] Specular/Reflections
- [ ] Russian Roulette
- [ ] Texture Sampling
- [ ] Allow for changing lighting models at runtime (Recompile shader with defines)
- [ ] EA Mapping
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
- Check for memory leaks, there's probably a lot

## PT-TODO:

### The Big Debug Feature Expansion

- [x] Readback pixel value
- [x] Output Buffers
- [x] Furnace Test
- [x] Change PathTracer to render into a local buffer which debug can modify/use/etc which then gets copied/rendered to the back buffer
- [x] Plot readback data into histogram (I think I should write to a file -> Execute python script -> Use Bokeh)
- [x] Fix whatever makes the furnace test fail
- [ ] RMSE Convergence Graph 

## Texture Array

The shader contains as its last SRV
Texture2D<float4> gTextures[] : register(tK);

The Heap on the CPU side now needs to have all DDS textures consecutive 
gTextures gets set to the base of the DDS textures
PtMaterialData.TextureIdx = Heap.IndexOf(ddsTex) - Heap.BaseDDS;

Don't call them DDS in code, find a better name. Material Textures?
Don't forget the sampler

Heap:
CBVs
UAVs
SRVs (TLAS, StructuredBuffers, etc)
SRVs (DDS Textures) <-- Base

Ez Pz let's do this