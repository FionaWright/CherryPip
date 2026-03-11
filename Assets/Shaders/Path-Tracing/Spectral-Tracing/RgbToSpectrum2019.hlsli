#ifndef H_RGB_TO_SPECTRUM_2019_H
#define H_RGB_TO_SPECTRUM_2019_H

float ReflectanceRgbToSpectrumSample(float3 rgb, float lambda);
float IlluminantRgbToSpectrumSample(float3 rgb, float lambda);

// TODO: Implement this and see if that works
// https://graphics.geometrian.com/research/spectral-primaries.html
// https://github.com/geometrian/simple-spectral
//float srgbToSpectrum(vec3 color, float wavelength) {
//    return dot(color, texelFetch(CIE_BT709_Basis, int(clamp(wavelength - 390.0, 0.0, 390.0)), 0).rgb);
//}

// https://rgl.epfl.ch/publications/Jakob2019Spectral

#include "Path-Tracing/Spectral-Tracing/Spectrum.hlsli"
#include "Path-Tracing/Spectral-Tracing/SpectralData/CIE2006.hlsli"

// Reflectance Spectrum:
// Incoming light reflected at each wavelength [0, 1]
// Independent of lighting
// Used for diffuse albedo, specular reflectance, material base colors, etc

// Illuminant Spectrum:
// Light energy emitted at each wavelength [0, infinity)
// Used for lamps, env maps, emissive materials, etc

void Spectrum::ReflectanceRgbToSpectrum(float3 rgb)
{
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
    {
        float lambda = IndexToLambda(i);
        float fIdx = (lambda - CIE_BASIS_LAMBDA_MIN) / (float)CIE_BASIS_LAMBDA_DELTA;
        int i0 = floor(fIdx);
        int i1 = i0 + 1;
        i0 = clamp(i0, 0, 390);
        i1 = clamp(i1, 0, 390);

        float t = fIdx - i0;

        float3 energies = lerp(cCIE_BasisBT709[i0], cCIE_BasisBT709[i1], t);

        Samples[i] = max(0.0f, dot(energies, rgb));
    }
}

void Spectrum::IlluminantRgbToSpectrum(float3 rgb)
{
    ReflectanceRgbToSpectrum(rgb);
    Mul(WhiteSpectrum_D65());
}

float ReflectanceRgbToSpectrumSample(float3 rgb, float lambda)
{
    float fIdx = (lambda - CIE_BASIS_LAMBDA_MIN) / (float)CIE_BASIS_LAMBDA_DELTA;
    int i0 = floor(fIdx);
    i0 = clamp(i0, 0, CIE_BASIS_COUNT-1);
    int i1 = min(i0 + 1, CIE_BASIS_COUNT-1);

    float t = fIdx - i0;

    float3 energies = lerp(cCIE_BasisBT709[i0], cCIE_BasisBT709[i1], t);

    return max(0.0f, dot(energies, rgb));
}

float IlluminantRgbToSpectrumSample(float3 rgb, float lambda)
{
    float energy = ReflectanceRgbToSpectrumSample(rgb, lambda);
    float d65Sample = WhiteSpectrum_D65().Sample(lambda);
    return energy * d65Sample;
}

#endif