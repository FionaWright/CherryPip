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
- [ ] Importance Sampling
- [ ] MIS
- [ ] Oren-Nayers Lighting Model
- [ ] Microfacet Beckmann (PBRT)
- [ ] Microfacet Trowbridge-Reitz (PBRT)
- [x] Firefly threshold
- [ ] Ray pipelines
- [ ] Spectral Path-Tracing 
- [ ] Rayleigh scattering
- [x] GBuffer pre-pass
- [x] Denoising (Box/Gauss/A-Trous)
- [x] Denoising Median
- [ ] Denoising NRD
- [ ] Maxwell "True" Fresnel (Requires eta/k tables but doesn't seem too hard)
- [ ] RMSE tests for different lighting models + denoisers
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

### Bugs (Physical Inaccuracies)
- Fix glass attenuation after, add attenuationDistance field

### Bugs (Classic)
- Camera movement gets messed up sometimes
- Env map rotations not paritied between EA/pano

### Features
- Mist (Randomly scatter rays depending on distance travelled, see SL video) (Rayleigh scattering! Requires spectral renderer?)
- PT only build mode that doesn't initialize any raster resources? 

### Misc
- Get a big scene that isn't bistro (Junkyard?)
- FPS has heavily dropped since I was away, possibly due to all the extra shaders/etc. Make sure they can be deleted properly when not in use
- Get a NICE picture for LinkedIn and post it for fun

## Raster/Laptop-TODO

- Mip maps aren't currently working. Need alternate solution for BC7 textures
- Stop irradiance cubemap making mip maps
- Bistro subfolder issue (Move all textures to root on build copy?)
- Beckmann for raster?
- Denoiser only initializes shaders/etc when needed (Easy refactor)
- Bindless all tex for raster to simplify, avoid creating 2 descriptors for each
- Refactor PtMaterialData to just MaterialData and share between backends
- Camera movement isn't framerate independent for some reason (WASD)

## Spectral Tracing

Surface/Volume interactions stay mostly the same, but you use wavelengths instead of colors
First test: Do a color roundtrip, reconstruct an srgb image using random samples in the spectral domain. Then convert to CIE and srgb. When the error is less than 1 ppm then it's good.
Requires rewriting the shaders
Hero model doesn't work with transmission (Glass, clouds, etc). This is because it assumes scatttering has no wavelength dependency 

Put all spectral notes into a new file, this might get large

See:
https://github.com/ashpil/moonshine 