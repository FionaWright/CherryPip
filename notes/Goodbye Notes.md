# Goodbye Notes

Final notes created before (hopefully temporarily) shelving the project

## TODO:

### Easy

- [ ] (★★★) NEE 
- [ ] (★★★) MIS 
- [ ] (★★★) Volumes
- [ ] (★★☆) Point Lights
- [ ] (★★☆) Thin-Film Interference
- [ ] (★★☆) Film-Simulation
- [ ] (★☆☆) Sobel RNG Sampler
- [ ] (★☆☆) Blackbody Emission (Glowing-hot objects)
- [ ] (★☆☆) Rayleigh scattering 
- [ ] (★☆☆) Mip-Maps (Requires refactoring BC7 system)

- [ ] (★★☆) (Unfinished) BTDF Microfacets

### Tough

- [ ] (★★★) Temporal denoising / SVGF
- [ ] (★★★) BSSDFs 
- [ ] (★★☆) Water
- [ ] (★★☆) Ray Marching -> Black Holes
- [ ] (★★☆) GPU Memory Profiler 
- [ ] (★★☆) Microfacet Beckmann
- [ ] (★★☆) SpecularTint, Clearcoat, Sheen
- [ ] (★★☆) Layered BSDFs
- [ ] (★☆☆) Denoising NRD
- [ ] (★☆☆) Oren-Nayers Lighting Model
- [ ] (★☆☆) Ray Bundles / Ray Buffers
- [ ] (★☆☆) Diffraction (Iridescence, CDs)
- [ ] (★☆☆) Lens simulation

### Sub-Projects

- [ ] (★★★) Gaussian Splatting
- [ ] (★★★) Resevoir Light Sampling / ReSTIR
- [ ] (★★☆) Ray pipelines
- [ ] (★★☆) Metropolis Light Transport
- [ ] (★★☆) Hair
- [ ] (★★☆) Neural Denoising (From scratch)
- [ ] (★★☆) Photon Mapping
- [ ] (★☆☆) Fluorescence + Neon
- [ ] (★☆☆) Polarized Rendering
- [ ] (★☆☆) Hybrid raster/path-tracing backend
- [ ] (★☆☆) Fluid simulation
- [ ] (★☆☆) Rigidbody simulation

### Refactors

- [ ] (★★★) Completely redo the GLTF loading systems
- [ ] (★★☆) Root Constants
- [ ] (★★☆) Combine RGB and Spectral renderers
- [ ] (★☆☆) Shared material buffer between backends and remove MaterialData/PtMaterialData distinction 
- [ ] (★☆☆) Move debug classes to SceneStudio

### Bugs

- [ ] (★★★) Camera movement gets messed up sometimes
- [ ] (★★★) Fix Bistro model transforms
- [ ] (★★☆) Halton sampler isn't as optimal as it should be
- [ ] (★☆☆) Env map rotations not paritied between EA/pano
- [ ] (★☆☆) Spectral: Very very specific angles on glass cause fresnel issues