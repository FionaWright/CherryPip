#ifndef H_SPECTRUM_TO_RGB2019_H
#define H_SPECTRUM_TO_RGB2019_H

float3 SpectrumToXYZ(Spectrum spectrum);
float3 SpectrumToRGB(Spectrum spectrum);

#include "Spectral-Tracing/Spectrum/Spectrum.hlsli"
#include "Spectral-Tracing/Spectrum/SpectralUtils.hlsli"
#include "Spectral-Tracing/SpectralData/CIE2006.hlsli"

float3 SampleCIE(float lambda)
{
    float fIdx = (lambda - CIE_XYZBAR_LAMBDA_MIN) / CIE_XYZBAR_LAMBDA_DELTA;
    fIdx = clamp(fIdx, 0.0f, CIE_XYZBAR_COUNT-1);
    int i0 = (int)floor(fIdx);
    int i1 = min(i0+1, CIE_XYZBAR_COUNT-1);
    float t = fIdx - i0;
    return lerp(cCIE_XYZbar[i0], cCIE_XYZbar[i1], t);
}

static const float cCIE_Y_integral = 118.5180915321789;

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
    static const float3x3 cMatXyzToRgb =
    {
        3.240479f, -1.537150f, -0.498535f,
        -0.969256f, 1.875991f, 0.041556f,
        0.055648f, -0.204043f, 1.057311f
    };

    float3 xyz = SpectrumToXYZ(spectrum);
    float3 rgb = mul(cMatXyzToRgb, xyz);
    return rgb;
}

float3 SpectrumSampleToXYZ(float energy, float lambda, float pdf)
{
    float3 cie = SampleCIE(lambda);
    return energy * cie / cCIE_Y_integral / max(1e-6f, pdf);
}

float3 SpectrumSampleToRGB(float energy, float lambda, float pdf)
{
    static const float3x3 cMatXyzToRgb =
    {
        3.240479f, -1.537150f, -0.498535f,
        -0.969256f, 1.875991f, 0.041556f,
        0.055648f, -0.204043f, 1.057311f
    };

    float3 xyz = SpectrumSampleToXYZ(energy, lambda, pdf);
    float3 rgb = mul(cMatXyzToRgb, xyz);
    return rgb;
}

#endif