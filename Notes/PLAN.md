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
- [x] Basic Ray-Tracer (Ray-Query)
- [ ] Furnace Test
- [x] Accumulation Buffer
- [ ] PT Debug Views & Systems
- [ ] CI/CD
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
- Consider dropping tangent/bitangent from attribute data (Needs CI test to see if it's faster)

## PT-TODO:

### The Big Debug Feature Expansion

- [ ] Readback pixel value
- [ ] Output Buffers
- [ ] Furnace Test
- [ ] RMSE Convergence Graph

## Notes

Look into russian roulette 