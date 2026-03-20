#ifndef H_RGB_TO_SPECTRUM_2019_H
#define H_RGB_TO_SPECTRUM_2019_H

float ReflectanceRgbToSpectrumSample(float3 rgb, float lambda);
float IlluminantRgbToSpectrumSample(float3 rgb, float lambda);

// https://graphics.geometrian.com/research/spectral-primaries.html
// https://github.com/geometrian/simple-spectral

// https://rgl.epfl.ch/publications/Jakob2019Spectral

#include "Spectral-Tracing/Spectrum/Spectrum.hlsli"
#include "Spectral-Tracing/SpectralData/CIE2006.hlsli"
#include "Spectral-Tracing/SpectralData/CIE2006_Cheb.hlsli"
#include "Spectral-Tracing/Spectrum/ColorSpectrums.hlsli"
#include "Spectral-Tracing/SpectralContext/SpectralContext.hlsli"

// Reflectance Spectrum:
// Incoming light reflected at each wavelength [0, 1]
// Independent of lighting
// Used for diffuse albedo, specular reflectance, material base colors, etc

// Illuminant Spectrum:
// Light energy emitted at each wavelength [0, infinity)
// Used for lamps, env maps, emissive materials, etc

float3 SampleCIEBasis(float lambda)
{
    float fIdx = (lambda - CIE_BASIS_LAMBDA_MIN) / (float)CIE_BASIS_LAMBDA_DELTA;
    int i0 = floor(fIdx);
    int i1 = i0 + 1;
    i0 = clamp(i0, 0, CIE_BASIS_COUNT-1);
    i1 = clamp(i1, 0, CIE_BASIS_COUNT-1);
    float t = fIdx - i0;
    return lerp(cCIE_BasisBT709[i0], cCIE_BasisBT709[i1], t);
}

float3 SampleCIEBasis_Chebyshev(float lambda)
{
    //float x = cCIE_BasisBT709_Cheb_X.Sample(lambda);
    float x = cCIE_BasisBT709_ChebPiecewise_X.Sample(lambda);
    //float y = cCIE_BasisBT709_Cheb_Y.Sample(lambda);
    float y = cCIE_BasisBT709_ChebPiecewise_Y.Sample(lambda);
    //float z = cCIE_BasisBT709_Cheb_Z.Sample(lambda);
    float z = cCIE_BasisBT709_ChebPiecewise_Z.Sample(lambda);
    //return float3(0, 0, SampleCIEBasis(lambda).z);
    //return float3(0, 0, z);
    //return float3(0, 0, cCIE_BasisBT709_Cheb_Z.Sample(lambda));
    //return float3(0, y, 0);
    //return float3(0, SampleCIEBasis(lambda).y, 0);
    return float3(x, y, z);
}

void Spectrum::ReflectanceRgbToSpectrum(float3 rgb)
{
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
    {
        float lambda = IndexToLambda(i);
        float3 basis = SampleCIEBasis(lambda);
        Samples[i] = max(0.0f, dot(basis, rgb));
    }
}

void Spectrum::IlluminantRgbToSpectrum(float3 rgb)
{
    ReflectanceRgbToSpectrum(rgb);
    Mul(WhiteSpectrum_D65());
}

float ReflectanceRgbToSpectrumSample(float3 rgb, float lambda)
{
    float3 basis = SampleCIEBasis_Chebyshev(lambda);
    return max(0.0f, dot(basis, rgb));
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
        float3 basis = SampleCIEBasis_Chebyshev(lambda);
        Samples[i] = max(0.0f, dot(basis, rgb));
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