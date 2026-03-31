#ifndef H_SPECTRAL_UTILS_H
#define H_SPECTRAL_UTILS_H

// Spectral Tracer Overview:
// https://simonstechblog.blogspot.com/2020/07/spectral-path-tracer.html

// sRGB to Spectrums:
// https://graphics.geometrian.com/research/spectral-primaries.html

// PBRT spectrum.cpp/h:
// https://github.com/mmp/pbrt-v3/blob/master/src/core/spectrum.cpp
// https://github.com/mmp/pbrt-v3/blob/master/src/core/spectrum.h

#include "Spectral-Tracing/Spectrum/RgbToSpectrum2019.hlsli"
#include "Spectral-Tracing/Spectrum/SpectrumToRGB2019.hlsli"
#include "Spectral-Tracing/Spectrum/ColorSpectrums.hlsli"
#include "Spectral-Tracing/SpectralValue/SpectralValue.hlsli"
#include "Spectral-Tracing/Spectrum/SpectrumUtils.hlsli"
#include "Random.h"

float SampleVisibleWavelength(inout float rngState)
{
    // TODO: Importance Sampling? Halton?
    float u = min(PcgRand01(rngState), 0.999999f);
    return lerp((float)VISIBLE_LIGHT_SPECTRUM_MIN, (float)VISIBLE_LIGHT_SPECTRUM_MAX, u);
}

float RgbToSpectrumSample(float3 rgb, SpectrumType type, float lambda)
{
    if (type == eReflectance)
        return ReflectanceRgbToSpectrumSample(rgb, lambda);
    else if (type == eIlluminant)
        return IlluminantRgbToSpectrumSample(rgb, lambda);

    return -1;
}

float BlackbodyRadiance(float lambda, float temp)
{
    if (temp <= 0)
        return 0;

    const float c = CONSTANT_SPEED_OF_LIGHT;
    const float h = CONSTANT_PLANK;
    const float kb = CONSTANT_BOLTZMANN;

    float l = lambda * 1e-9f; // nm -> m
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

float PlanksLaw(float lambda, float temp)
{
	float lambda_m = lambda * 1.0e-9f;

	//First radiation constant "2 h c²"
	float c_1L = 2.0f * CONSTANT_PLANK * CONSTANT_SPEED_OF_LIGHT*CONSTANT_SPEED_OF_LIGHT;
	//Second radiation constant "h c / k_B"
	float c_2  =  CONSTANT_PLANK * CONSTANT_SPEED_OF_LIGHT / CONSTANT_BOLTZMANN;

	float denom = pow(lambda_m, 5.0f) * (exp( c_2 / (lambda_m*temp) ) - 1.0f);
	float value = c_1L / denom;
	return value * 1.0e-9f; // m -> km
}

#endif