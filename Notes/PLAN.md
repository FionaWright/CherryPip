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
- [ ] NEE 
- [x] Glass model
- [x] Importance Sampling
- [ ] MIS
- [ ] Proper distant light using bounding sphere of plane (No singular sun-point) (PBRT)
- [ ] Oren-Nayers Lighting Model (https://blog.selfshadow.com/publications/s2012-shading-course/gotanda/s2012_pbs_beyond_blinn_slides_v3.pdf)
- [ ] Microfacet Beckmann (PBRT)
- [ ] Microfacet Glass (Beckmann)
- [ ] Point Lights
- [ ] SpecularTint, Clearcoat, Sheen PBR parameters
- [x] Firefly threshold
- [ ] BTDFs (VNDF whitepaper has some good stuff)
- [ ] Sobel RNG Sampler
- [ ] Ray pipelines
- [ ] Spectral Path-Tracing 
- [ ] Rayleigh scattering (Mist, See Seb Lague)
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
- Consider dropping tangent/bitangent from attribute data (Needs CI test to see if it's faster)
- Check for memory leaks, there's probably a lot 
- Add support for root constants in Material and RootSig and apply to ATrous filter
- Temporary heap which gets cleared at the end of each frame? Or maybe just clear all unused data from heaps every few frames

## PT-TODO:

### Main Tasks
- Fix Anisotropic issues
- Fix VNDF issues
- Fix Firefly Threshold

### Bugs (Physical Inaccuracies)
- Fix glass attenuation after, add attenuationDistance field
- Dir light is wrong direction, see reflection in metalRough spheres. Disable bounce threshold to see easier. Make only samplable for diffuse rays?
- Firefly threshold not helping, fix darkness issue first

### Bugs (Classic)
- Camera movement gets messed up sometimes
- Env map rotations not paritied between EA/pano
- Fix Bistro model transforms

### Features
- PT only build mode that doesn't initialize any raster resources? 

### Misc
- FPS has heavily dropped since I was away, possibly due to all the extra shaders/etc. Make sure they can be deleted properly when not in use. 1000+ FPS with lambert distribution should be possible
- Class refactor to clean up shader code

## Raster/Laptop-TODO

- Mip maps aren't currently working. Need alternate solution for BC7 textures
- Beckmann for raster?
- Refactor PtMaterialData to just MaterialData and share between backends
- Irradiance prealiasing, convert discrete to continuous through c = d + 0.5? Does this apply to unjittered PT samples as well?
- Something wrong with sphere model, makes furnace tests hard
- Bindless heap sharing
- Line drawing debug tool (Use for dir light and axes) Then use to fix dir light dir

## Class Refactor

- MicrofacetModel Superclass
- GgxSmith, GgxVCavity_VNDF, GgxSmith_VNDF, Beckmann, Beckmann_VNDF Subclasses
- RoughnessToAlpha, SampleIso, SampleAniso, D, G1, G, PDF functions
- Make new MicrofacetModels folder and put Microfacet.hlsli and each subclass into their own .hlsli file
- Leave anything shared/general in Microfacet.hlsi, maybe rename file once I know what's left in there
- Goal: Model_Microfacet should be clean af