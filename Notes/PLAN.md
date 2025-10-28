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
- [ ] Clean up CMAKE
- [ ] Basic Ray-Tracer (Ray-Query)
- [ ] Furnace Test
- [ ] PT Debug Views & Systems
- [ ] CI/CD
- [ ] GPU-Accelerated BVH
- [ ] ...
- [ ] Reflections
- [ ] Glass & Caustics
- [ ] Ray pipelines

https://learn.microsoft.com/en-us/samples/microsoft/directx-graphics-samples/d3d12-raytracing-samples-win32/

## Optimizations-TODO:

- Frustum Culling
- Texture/Model reuse
- Descriptor heap sharing (Wait to see if it's worth it)
- PCH Compilation? 

## PT-TODO:

- Share InstanceData/Vertex
- New PathTracer class that contains FullScreenTriangle, TLAS and structured vertex/index buffer buffer and instanceData buffer. (PathTracingContext as a name?)
- Create structured buffers and pass them in 
