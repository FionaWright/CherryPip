#ifndef H_RGB_TO_SPECTRUM_2019_H
#define H_RGB_TO_SPECTRUM_2019_H

float ReflectanceRgbToSpectrumSample(float3 rgb, float lambda);
float IlluminantRgbToSpectrumSample(float3 rgb, float lambda);

// https://graphics.geometrian.com/research/spectral-primaries.html
// https://github.com/geometrian/simple-spectral

// https://rgl.epfl.ch/publications/Jakob2019Spectral

#include "Spectral-Tracing/Spectrum/Spectrum.hlsli"
#include "Spectral-Tracing/SpectralData/CIE2006.hlsli"
#include "Spectral-Tracing/Spectrum/ColorSpectrums.hlsli"
#include "Spectral-Tracing/SpectralContext/SpectralContext.hlsli"

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
    float d65Sample = SampleD65_MLG(lambda);
    return energy * d65Sample;
}

#ifdef SPECTRAL_HERO_SAMPLING
#include "Spectral-Tracing/Spectrum/HeroSpectrum.hlsli"
void HeroSpectrum::ReflectanceRgbToSpectrum(float3 rgb, SpectralContext ctx)
{
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
    {
        float lambda = ctx.GetLambda(i);
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

void HeroSpectrum::IlluminantRgbToSpectrum(float3 rgb, SpectralContext ctx)
{
    ReflectanceRgbToSpectrum(rgb, ctx);

    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
    {
        float lambda = ctx.GetLambda(i);
        float d65 = SampleD65_MLG(lambda);
        Samples[i] *= d65;
    }
}
#endif

#endif