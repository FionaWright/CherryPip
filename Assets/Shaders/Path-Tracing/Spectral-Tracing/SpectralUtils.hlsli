#ifndef H_SPECTRAL_UTILS_H
#define H_SPECTRAL_UTILS_H

// Spectral Tracer Overview:
// https://simonstechblog.blogspot.com/2020/07/spectral-path-tracer.html

// sRGB to Spectrums:
// https://graphics.geometrian.com/research/spectral-primaries.html

// PBRT spectrum.cpp/h:
// https://github.com/mmp/pbrt-v3/blob/master/src/core/spectrum.cpp
// https://github.com/mmp/pbrt-v3/blob/master/src/core/spectrum.h

// Is controlling the temperature of the scene something I can use?

#include "Spectrum.h"
#include "Path-Tracing/Spectral-Tracing/SpectrumToXYZ.hlsli"
#include "Random.h"

Spectrum BlackSpectrum()
{
    Spectrum spectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        spectrum.Samples[i] = 0.0f;
    return spectrum;
}

// Doesn't have to be 1.0f, just uniform to be white
Spectrum WhiteSpectrum()
{
    Spectrum spectrum;
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        spectrum.Samples[i] = 1.0f;
    return spectrum;
}

float SampleVisibleWavelength(inout float rngState)
{
    // TODO: Importance Sampling? Halton?
    return VISIBLE_LIGHT_SPECTRUM_MIN + PcgRand01(rngState) * VISIBLE_LIGHT_SPECTRUM_SIZE;
}

float ExtractFromSpectrum(Spectrum spectrum, float wavelength)
{
    float lambda01 = (wavelength - VISIBLE_LIGHT_SPECTRUM_MIN) / VISIBLE_LIGHT_SPECTRUM_SIZE;
    float idx = lambda01 * (NUM_SPECTRUM_SAMPLES - 1);
    uint floorIdx = floor(idx);
    float lowerSample = spectrum.Samples[floorIdx];
    float upperSample = spectrum.Samples[min(floorIdx+1, NUM_SPECTRUM_SAMPLES - 1)];
    float t = idx - (float)floorIdx;
    return lerp(lowerSample, upperSample, t);
}

float3 SpectrumToRGB(Spectrum spectrum)
{
    // Convert to XYZ using CIE curves and DeltaLambda
    // CIE 1931 recommended
    // Multiply by the XYZ->RGB matrix
    // Apply gamma correction for RGB -> sRGB

    static const float3x3 cMatXyzToRgb =
    {
        3.240479f, -1.537150f, -0.498535f,
        -0.969256f, 1.875991f, 0.041556f,
        0.055648f, -0.204043f, 1.057311f
    };

    float3 xyz = SpectrumToXYZ(spectrum);
    float3 rgb = mul(cMatXyzToRgb, xyz);
    //float3 sRGB = pow(rgb, 1.0f / 2.2f);
    return rgb;
}

float BlackbodyRadiance(float lambda, float temp)
{
    if (temp <= 0)
        return 0;

    const float c = 299792458.f; // Speed of light
    const float h = 6.62606957e-34f; // Planks Constant
    const float kb = 1.3806488e-23f; // Boltzmann Constant

    float l = lambda * 1e-9f; // Convert nm to meters
    float l5 = pow(l, 5);
    float e = exp(h * c / (l * kb * temp));
    float Le = 2 * h * c * c / (l5 * (e - 1));
    return Le;
}

float EmittedRadiance(float lambda, float temp, float w) // What is w?
{
    float rho_hd = w; // ?
    return BlackbodyRadiance(lambda, temp) * (1 - rho_hd);
}

#endif