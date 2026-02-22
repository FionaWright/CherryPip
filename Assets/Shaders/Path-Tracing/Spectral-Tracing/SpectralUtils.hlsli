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

#include "Path-Tracing/Spectral-Tracing/Spectrum.hlsli"
#include "Random.h"

#include "Path-Tracing/Spectral-Tracing/ColorSpectrums.hlsli"

float LambdaToIndex(float lambda)
{
    float fIdx = (lambda - VISIBLE_LIGHT_SPECTRUM_MIN) / VISIBLE_LIGHT_SPECTRUM_SIZE;
    fIdx *= (NUM_SPECTRUM_SAMPLES-1);
    return fIdx;
}

float IndexToLambda(float idx)
{
    return VISIBLE_LIGHT_SPECTRUM_MIN + (idx * SPECTRUM_DELTA_LAMBDA);
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