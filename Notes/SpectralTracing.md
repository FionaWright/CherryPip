# Spectral Tracing Notes

Every ray gets assigned a random wavelength from the visible light spectrum and monte carlo integration is used to average over the entire spectrum
Colors are no longer stored as RGB float3s. They are instead all stored and used as spectrums
A good way of storing spectrums is to make them arrays of floats representing intensity at 30-40 sample points along 380-780nm
`float spectrum[NUM_SAMPLES]` 
Dielectrics and metals need refractive indices per wavelength 
Light sources are no longer a single float3, instead uses measured SPDs. For emissive materials it uses a spectrum as well 
The rays wavelength is input into the spectrum of a material (albedo, etc) to get the intensity 

After path integration the spectrum needs to be converted to RGB for display 
Multiply it by the CIE color matching functions, integrate over wavelengths to get XYZ tristimulus values then convert XYZ to sRGB

## Hero-Wavelength Sampling

This is an extension you can add later that allows rays to hold multiple wavelengths at the cost of preventing transmission 