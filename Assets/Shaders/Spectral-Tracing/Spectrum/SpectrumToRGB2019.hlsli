#ifndef H_SPECTRUM_TO_RGB2019_H
#define H_SPECTRUM_TO_RGB2019_H

float3 SpectrumToXYZ(Spectrum spectrum);
float3 SpectrumToRGB(Spectrum spectrum);
float3 HeroToXYZ(Spectrum spectrum, SpectralContext ctx);
float3 HeroToRGB(Spectrum spectrum, SpectralContext ctx);

#include "Spectral-Tracing/Spectrum/Spectrum.hlsli"
#include "Spectral-Tracing/SpectralUtils.hlsli"
#include "Spectral-Tracing/Spectrum/HeroSpectrum.hlsli"
#include "Spectral-Tracing/SpectralData/CIE2006.hlsli"
#include "Spectral-Tracing/SpectralData/CIE2006_MLG.hlsli"

static const float3x3 cMatXyzToRgb =
    {
        3.240479f, -1.537150f, -0.498535f,
        -0.969256f, 1.875991f, 0.041556f,
        0.055648f, -0.204043f, 1.057311f
    };

// Better for Full-Spectrum, values are hardcoded at comptime
float3 SampleCIE(float lambda)
{
    float fIdx = (lambda - CIE_XYZBAR_LAMBDA_MIN) / CIE_XYZBAR_LAMBDA_DELTA;
    fIdx = clamp(fIdx, 0.0f, CIE_XYZBAR_COUNT-1);
    int i0 = (int)floor(fIdx);
    int i1 = min(i0+1, CIE_XYZBAR_COUNT-1);
    float t = fIdx - i0;
    return lerp(cCIE_XYZbar[i0], cCIE_XYZbar[i1], t);
}

// Better for spectral sampling modes that don't know lambda at comptime
float3 SampleCIE_MLG(float lambda)
{
    float normalizedLambda = (lambda - VISIBLE_LIGHT_SPECTRUM_MIN) / float(CIE_XYZBAR_LAMBDA_SIZE);
    float x = cCIE_XYZbar_MLG_X.Sample(normalizedLambda);
    float y = cCIE_XYZbar_MLG_Y.Sample(normalizedLambda);
    float z = cCIE_XYZbar_MLG_Z.Sample(normalizedLambda);
    return float3(x, y, z);
}

// Reimann Sum Approximation
float3 SpectrumToXYZ(Spectrum spectrum)
{
    float3 xyz = 0.0f;

    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
    {
        float lambda = IndexToLambda((float)i);
        float3 cie = SampleCIE(lambda);
        xyz += spectrum.Samples[i] * cie;
    }

    // Normalization
    xyz *= (float)SPECTRUM_DELTA_LAMBDA;
    xyz /= cCIE_Y_integral;
    return xyz;
}

float3 SpectrumToRGB(Spectrum spectrum)
{
    float3 xyz = SpectrumToXYZ(spectrum);
    float3 rgb = mul(cMatXyzToRgb, xyz);
    return rgb;
}

float3 SpectrumSampleToXYZ(float energy, float lambda, float pdf)
{
    float3 cie = SampleCIE_MLG(lambda);
    return energy * cie / cCIE_Y_integral / max(1e-6f, pdf);
}

float3 SpectrumSampleToRGB(float energy, float lambda, float pdf)
{
    float3 xyz = SpectrumSampleToXYZ(energy, lambda, pdf);
    float3 rgb = mul(cMatXyzToRgb, xyz);
    return rgb;
}

#ifdef SPECTRAL_HERO_SAMPLING
float3 HeroToXYZ(HeroSpectrum spectrum, SpectralContext ctx)
{
    float3 xyz = 0.0f;

    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
    {
        float lambda = ctx.GetLambda(i);
        float3 cie = SampleCIE_MLG(lambda);
        xyz += spectrum.Samples[i] * cie;
    }

    // Normalization
    xyz /= cCIE_Y_integral;
    xyz /= max(1e-6f, ctx.GetPDF());
    xyz /= float(NUM_HERO_SAMPLES);
    return xyz;
}

float3 HeroToRGB(HeroSpectrum spectrum, SpectralContext ctx)
{
    float3 xyz = HeroToXYZ(spectrum, ctx);
    float3 rgb = mul(cMatXyzToRgb, xyz);
    return rgb;
}
#endif

#endif