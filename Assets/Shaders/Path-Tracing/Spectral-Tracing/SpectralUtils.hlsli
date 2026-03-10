#ifndef H_SPECTRAL_UTILS_H
#define H_SPECTRAL_UTILS_H

// Spectral Tracer Overview:
// https://simonstechblog.blogspot.com/2020/07/spectral-path-tracer.html

// sRGB to Spectrums:
// https://graphics.geometrian.com/research/spectral-primaries.html

// PBRT spectrum.cpp/h:
// https://github.com/mmp/pbrt-v3/blob/master/src/core/spectrum.cpp
// https://github.com/mmp/pbrt-v3/blob/master/src/core/spectrum.h

float LambdaToIndex(float lambda);
float IndexToLambda(float idx);

#include "Path-Tracing/Spectral-Tracing/Spectrum.hlsli"
#include "Path-Tracing/Spectral-Tracing/SpectrumToRGB2019.hlsli"
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

// For Debugging
float3 WavelengthToRGB(float wavelength)
{
    float r=0, g=0, b=0;

    if (wavelength >= 380 && wavelength < 440) 
    {
        r = -(wavelength - 440) / (440 - 380);
        g = 0;
        b = 1;
    }
    else if (wavelength < 490) 
    {
        r = 0;
        g = (wavelength - 440) / (490 - 440);
        b = 1;
    }
    else if (wavelength < 510) 
    {
        r = 0;
        g = 1;
        b = -(wavelength - 510) / (510 - 490);
    }
    else if (wavelength < 580) 
    {
        r = (wavelength - 510) / (580 - 510);
        g = 1;
        b = 0;
    }
    else if (wavelength < 645)
    {
        r = 1;
        g = -(wavelength - 645) / (645 - 580);
        b = 0;
    }
    else if (wavelength <= 780) 
    {
        r = 1;
        g = 0;
        b = 0;
    }

    float factor;
    if (wavelength < 420)
        factor = 0.3 + 0.7*(wavelength-380)/(420-380);
    else if (wavelength <= 700)
        factor = 1.0;
    else
        factor = 0.3 + 0.7*(780-wavelength)/(780-700);

    return float3(r,g,b) * factor;
}

#define CONSTANT_BOLTZMANN 1.38064852e-23f
#define CONSTANT_PLANK 6.62607015e-34f
#define CONSTANT_SPEED_OF_LIGHT 299792458.0f

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

float3 RoundTripTest(float3 lrgb)
{
    Spectrum s;

    Spectrum whiteD65 = WhiteSpectrum_D65();

    float kelvinD65 = 6500.0f;
    kelvinD65 *= (CONSTANT_PLANK * CONSTANT_SPEED_OF_LIGHT / CONSTANT_BOLTZMANN) / 1.438e-2f;
    //whiteD65.Mul(0.001f * PlanksLaw(560, kelvinD65)); // Original -> Radiance

    s.InitFromRGB(lrgb, eReflectance);
    s.Mul(whiteD65); // Reflectance -> Radiance

    return SpectrumToRGB(s);
}

#endif