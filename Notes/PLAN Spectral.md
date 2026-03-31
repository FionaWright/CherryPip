# Spectral Plan

## TODO

- [x] Fix gamma correction
- [x] Refactor SpectralValue system, operator overloading? Prepare for Hero sampling and possibly RGB. There should be no ifdefs in Trace/Hit code
- [x] Hero wavelength sampling
- [x] Finish reading PBRT section on specular
- [x] Implement proper BTDF transmission into the path-tracer first, deprecate glass model?
- [x] Fix Fresnel + BTDF
- [x] Add conductors to fresnel
- [x] Implement other random stuff like Russian Roulette, etc
- [ ] Rayleigh/Mie scattering, Mist
- [x] Add curve fitting to CIE basis
- [ ] Fix hero sampling after implementing/understanding MIS
- [ ] Thin-film interference
- [ ] Diffraction (Iridescence, CDs)
- [ ] Fluorescence + Neon

## Note:

There doesn't seem to be great online resources/knowledge of how to effectively curve fit various spectral curves. IOR Spectra has Sellmeier as a common standard but CIE XYZ, CIE Basis, D65 all are kinda unknown? 