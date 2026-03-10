#ifndef H_RGB_TO_SPECTRUM_H
#define H_RGB_TO_SPECTRUM_H

#include "Path-Tracing/Spectral-Tracing/Spectrum.hlsli"
#include "Path-Tracing/Spectral-Tracing/Spectra.hlsli"

// TODO: Implement this and see if that works
// https://graphics.geometrian.com/research/spectral-primaries.html
// https://github.com/geometrian/simple-spectral
//float srgbToSpectrum(vec3 color, float wavelength) {
//    return dot(color, texelFetch(CIE_BT709_Basis, int(clamp(wavelength - 390.0, 0.0, 390.0)), 0).rgb);
//}

// https://rgl.epfl.ch/publications/Jakob2019Spectral

// Reflectance Spectrum:
// Incoming light reflected at each wavelength [0, 1]
// Independent of lighting
// Used for diffuse albedo, specular reflectance, material base colors, etc

// Illuminant Spectrum:
// Light energy emitted at each wavelength [0, infinity)
// Used for lamps, env maps, emissive materials, etc

// TODO: Align curve lambdas with spectrum to allow easy component-wise multiplication?
// Multiply curve by mult, then add to spectrum
void Spectrum::FmaCurve(float curve[32], float mult)
{
    [unroll]
    for (int i = 0; i < 32; i++)
    {
        float lambda = cRGB2SpectLambda[i];
        float energy = curve[i] * mult;
        //float energy = curve[i] * mult * SPECTRUM_DELTA_LAMBDA;

		Samples[i] += energy;

//        float fIdx = LambdaToIndex(lambda);
//        int i0 = floor(fIdx);
//        int i1 = i0 + 1;
//
//        if (i0 < 0 || i1 >= NUM_SPECTRUM_SAMPLES)
//            continue;
//        //i0 = max(i0, 0);
//        //i1 = min(i1, NUM_SPECTRUM_SAMPLES-1);
//
//        float t = fIdx - i0;
//
//        Samples[i0] += (1.0f - t) * energy;
//        Samples[i1] += t * energy;
    }
}

void Spectrum::ReflectanceRgbToSpectrum(float3 rgb)
{
    float r = rgb.x;
    float g = rgb.y;
    float b = rgb.z;

    if (r <= g && r <= b)
    {
        // Compute reflectance _SampledSpectrum_ with _r_ as minimum
        FmaCurve(cRGBRefl2SpectWhite, r);
        if (g <= b)
        {
            FmaCurve(cRGBRefl2SpectCyan, g - r);
            FmaCurve(cRGBRefl2SpectBlue, b - g);
        }
        else
        {
            FmaCurve(cRGBRefl2SpectCyan, b - r);
            FmaCurve(cRGBRefl2SpectGreen, g - b);
        }
    }
    else if (g <= r && g <= b)
    {
        // Compute reflectance _SampledSpectrum_ with _g_ as minimum
        FmaCurve(cRGBRefl2SpectWhite, g);
        if (r <= b)
        {
            FmaCurve(cRGBRefl2SpectMagenta, r - g);
            FmaCurve(cRGBRefl2SpectBlue, b - r);
        }
        else
        {
            FmaCurve(cRGBRefl2SpectMagenta, b - g);
            FmaCurve(cRGBRefl2SpectRed, r - b);
        }
    }
    else
    {
        // Compute reflectance _SampledSpectrum_ with _b_ as minimum
        FmaCurve(cRGBRefl2SpectWhite, b);
        if (r <= g)
        {
            FmaCurve(cRGBRefl2SpectYellow, r - b);
            FmaCurve(cRGBRefl2SpectGreen, g - r);
        }
        else
        {
            FmaCurve(cRGBRefl2SpectYellow, g - b);
            FmaCurve(cRGBRefl2SpectRed, r - g);
        }
    }

    Mul(0.94f);
    Clamp(0.0f, 1.0f);
}

void Spectrum::IlluminantRgbToSpectrum(float3 rgb)
{
    float r = rgb.x;
    float g = rgb.y;
    float b = rgb.z;
    
    if (r <= g && r <= b)
    {
        // Compute illuminant _SampledSpectrum_ with _r_ as minimum
        FmaCurve(cRGBIllum2SpectWhite, r);
        if (g <= b)
        {
            FmaCurve(cRGBIllum2SpectCyan, g - r);
            FmaCurve(cRGBIllum2SpectBlue, b - g);
        }
        else
        {
            FmaCurve(cRGBIllum2SpectCyan, b - r);
            FmaCurve(cRGBIllum2SpectGreen, g - b);
        }
    }
    else if (g <= r && g <= b)
    {
        // Compute illuminant _SampledSpectrum_ with _g_ as minimum
        FmaCurve(cRGBIllum2SpectWhite, g);
        if (r <= b)
        {
            FmaCurve(cRGBIllum2SpectMagenta, r - g);
            FmaCurve(cRGBIllum2SpectBlue, b - r);
        }
        else
        {
            FmaCurve(cRGBIllum2SpectMagenta, b - g);
            FmaCurve(cRGBIllum2SpectRed, r - b);
        }
    }
    else
    {
        // Compute illuminant _SampledSpectrum_ with _b_ as minimum
        FmaCurve(cRGBIllum2SpectWhite, b);
        if (r <= g)
        {
            FmaCurve(cRGBIllum2SpectYellow, r - b);
            FmaCurve(cRGBIllum2SpectGreen, g - r);
        }
        else
        {
            FmaCurve(cRGBIllum2SpectYellow, g - b);
            FmaCurve(cRGBIllum2SpectRed, r - g);
        }
    }
    Mul(0.86445f);
    Clamp(0.0f, 1.0f);
}

#endif
