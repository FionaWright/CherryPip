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
- [ ] ~~Build shaders in cmake~~
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
- [x] Russian Roulette
- [x] DoF
- [x] Texture Sampling
- [x] Allow for changing lighting models at runtime (Recompile shader with defines)
- [x] EA Mapping
- [x] Raster skyboxes
- [x] Directional Lighting
- [ ] MIS
- [ ] NEE 
- [x] Glass model
- [x] Importance Sampling
- [x] Proper distant light using bounding sphere of plane (No singular sun-point) (PBRT)
- [ ] Oren-Nayers Lighting Model (https://blog.selfshadow.com/publications/s2012-shading-course/gotanda/s2012_pbs_beyond_blinn_slides_v3.pdf)
- [ ] Microfacet Beckmann (PBRT)
- [ ] Microfacet Glass
- [ ] Point Lights
- [ ] Ray Bundles / Ray Buffers(?). See Embre? (https://www.jp.square-enix.com/tech/library/pdf/Global%20Illumination%20Using%20Ray-Bundle%20Tracing%20(AFDS2012).pdf)
- [ ] SpecularTint, Clearcoat, Sheen PBR parameters
- [x] Firefly threshold
- [ ] BTDFs (VNDF whitepaper has some good stuff)
- [ ] Sobel RNG Sampler
- [ ] Ray pipelines
- [x] Spectral Path-Tracing 
- [ ] Rayleigh scattering (Mist, See Seb Lague)
- [ ] BSSDFs (Subsurface Scattering)
- [ ] Water
- [x] GBuffer pre-pass
- [x] Denoising (Box/Gauss/A-Trous)
- [x] Denoising Median
- [ ] Denoising NRD
- [ ] Maxwell "True" Fresnel (Requires eta/k tables but doesn't seem too hard)
- [x] RMSE tests for different lighting models + denoisers
- [ ] Resevoir Light Sampling
- [ ] ReSTIR
- [ ] GPU Memory Profiler (Track memory usage, find leaks/waste)

https://learn.microsoft.com/en-us/samples/microsoft/directx-graphics-samples/d3d12-raytracing-samples-win32/

## Optimizations-TODO:

- Frustum Culling
- Descriptor heap sharing (Wait to see if it's worth it)
- Check for memory leaks, there's probably a lot 
- Add support for root constants in Material and RootSig and apply to ATrous filter
- Temporary heap which gets cleared at the end of each frame? Or maybe just clear all unused data from heaps every few frames

## PT-TODO:

### Main Tasks
- Proper BTDF / BSDF system. (Transmission)
- THEN I can try again to implement the microfacet/glass model into spectral

### Bugs 
- Camera movement gets messed up sometimes
- Env map rotations not paritied between EA/pano
- Fix Bistro model transforms
- Release mode is probably broken, fix

### Misc
- PT only build mode that doesn't initialize any raster resources? 
- FPS has heavily dropped since I was away, possibly due to all the extra shaders/etc. Make sure they can be deleted properly when not in use. 1000+ FPS with lambert distribution should be possible
- Build basis with N and V when not aligned instead of using frisvad? 
- Try make Halton work better, reimplement for bounces
- Fix firefly threshold when I start noticing actual fireflies, none yet. Maybe after MIS or NEE?
- Debug: Ability to run two different versions of the PT at once, split-screen.
- Refactor TBN into a new struct called ShadingFrame with .ToLocal() sorta functions

## Raster/Laptop-TODO

- Mip maps aren't currently working. Need alternate solution for BC7 textures
- Beckmann for raster?
- Refactor PtMaterialData to just MaterialData and share between backends
- Irradiance prealiasing, convert discrete to continuous through c = d + 0.5? Does this apply to unjittered PT samples as well?
- Something wrong with sphere model, makes furnace tests hard
- Refactor some of the debug classes into the SceneStudio app
- Make every third-party thing fetched using cmake
- Make new test which renders nothing to the screen. Should be able to achieve 1000+ FPS. If not then optimize CPU side using profiling. (Plus test which renders white to the screen for each pixel but still uses each draw call)

## Notes

Remember that gamma correction is more complicated than a simple power. See Color.hpp from Mattel+Yuksel or whatever

Convert hard-coded spectral data to CBVs. It's slowing down hot reloading a ton.