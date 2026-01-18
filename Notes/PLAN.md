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
- [ ] DoF
- [x] Texture Sampling
- [x] Allow for changing lighting models at runtime (Recompile shader with defines)
- [x] EA Mapping
- [x] Raster skyboxes
- [ ] NEE 
- [x] Glass model
- [ ] Importance Sampling
- [ ] MIS
- [ ] Ray pipelines
- [ ] Spectral Path-Tracing (Hero, make separate from RGB PT)
- [ ] Rayleigh scattering
- [x] GBuffer pre-pass
- [x] Denoising (Box/Gauss/A-Trous)
- [x] Denoising Median
- [ ] Denoising NRD
- [ ] ReSTIR

https://learn.microsoft.com/en-us/samples/microsoft/directx-graphics-samples/d3d12-raytracing-samples-win32/

## Optimizations-TODO:

- Frustum Culling
- Texture/Model reuse
- Descriptor heap sharing (Wait to see if it's worth it)
- PCH Compilation? 
- Consider dropping tangent/bitangent from attribute data (Needs CI test to see if it's faster)
- Check for memory leaks, there's probably a lot
- Make only a few scenes part of the repo, larger ones are not included by default 
- Add support for root constants in Material and RootSig and apply to ATrous filter

## PT-TODO:

- Fix glass issue at grazing angles (Read PBRT section, stop using the goddamn GPT)
- Fix glass attenuation after, add attenuationDistance field
- Add stanford dragon model to test with and perform glass ball test with it (Object behind should be vertically flipped depending on distance)
- Mist (Randomly scatter rays depending on distance travelled, see SL video) (Rayleigh scattering! Requires spectral renderer?)
- Get dir light from env map
- Get a big scene that isn't bistro 

- Camera movement gets messed up sometimes

- FPS has heavily dropped since I was away, possibly due to all the extra shaders/etc. Make sure they can be deleted properly when not in use

## Raster-TODO

- Get proper BSDF for forward renderer
- Add deferred render backend
- GBuffer debug viewer

## Spectral Tracing

Surface/Volume interactions stay mostly the same, but you use wavelengths instead of colors
First test: Do a color roundtrip, reconstruct an srgb image using random samples in the spectral domain. Then convert to CIE and srgb. When the error is less than 1 ppm then it's good.
Requires rewriting the shaders
Hero model doesn't work with transmission (Glass, clouds, etc). This is because it assumes scatttering has no wavelength dependency 

Put all spectral notes into a new file, this might get large

See:
https://github.com/ashpil/moonshine 