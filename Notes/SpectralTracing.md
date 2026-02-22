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

## TODO:

- Make spectral tracer only run single frame at a time based on GUI input, or select checkbox to run forever
- Get basic lambertian working
- You may need to clean up SceneStudio a good bit to make it tidy
- Figure out how to use wavelength-per-path instead of working on entire spectrums
- See if you can optimize/pre-compute anything to boost speed
- Start re-implementing parts from the PT one by one
- Try create some rainbows/mist or something ST-exclusive 

## Thoughts:

What is up/down sampling? 
    Upsampling is converting RGB -> Spectrum
    Downsampling is converting Spectrum -> RGB

How do SPDs come into this? I think thats in my research somewhere 
    An SPD is just a radiant flux per wavelength spectrum. What I have already

What are refractive index spectra and how do I get them for each material?

I think you might need to divide by the wavelength PDF as well as the usual direction PDF

Remember that colors are photometric whereas spectra are radiometric, converting between them requires information about the observer (camera/eyeball/etc)

To convert from spectrum to XYZ color space, I integrate its product with the spectral matching curved X(l), Y(l), Z(l). I'll need a PDF and the integral of the Y(l) curve as well 
I might get the R(l), G(l), B(l) curves from the display? Need to extract info from the monitor? 
If you have the XYZ color you can convert straight to RGB using a simple matrix

I'll need to specify a whitepoint as well as the primaries, this is determined by the D65 illuminant 

Suspicious of multiplication between spectra being simple component-wise but...idk?

## CIE XYZ Output:

![alt text](image-1.png)