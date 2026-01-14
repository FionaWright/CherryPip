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
- [x] Scene Transforms/Materials GUI
- [ ] CI/CD-like tests
- [x] Specular/Reflections
- [ ] Russian Roulette
- [ ] DoF
- [x] Texture Sampling
- [x] Allow for changing lighting models at runtime (Recompile shader with defines)
- [ ] EA Mapping
- [ ] NEE 
- [x] Glass & Caustics
- [ ] Ray pipelines
- [ ] Spectral Path-Tracing (Hero, make separate from RGB PT)
- [ ] ReSTIR

https://learn.microsoft.com/en-us/samples/microsoft/directx-graphics-samples/d3d12-raytracing-samples-win32/

## Optimizations-TODO:

- Frustum Culling
- Texture/Model reuse
- Descriptor heap sharing (Wait to see if it's worth it)
- PCH Compilation? 
- Consider dropping tangent/bitangent from attribute data (Needs CI test to see if it's faster)
- Check for memory leaks, there's probably a lot

## PT-TODO:

- Fix glass issue at grazing angles (Read PBRT section, stop using the goddamn GPT)
- Fix glass attenuation after, add attenuationDistance field
- Add stanford dragon model to test with and perform glass ball test with it (Object behind should be vertically flipped depending on distance)
- Russian Roulette 
- Fix issue with hot reloading, env maps breaking on scene reload, camera movement being messed up (Is the screen flipped??)
- Mist (Randomly scatter rays depending on distance travelled, see SL video) (Rayleigh scattering! Requires spectral renderer?)
