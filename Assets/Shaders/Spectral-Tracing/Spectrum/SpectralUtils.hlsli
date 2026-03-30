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

#include "Spectral-Tracing/Spectrum/Spectrum.hlsli"
#include "Spectral-Tracing/Spectrum/RgbToSpectrum2019.hlsli"
#include "Spectral-Tracing/Spectrum/SpectrumToRGB2019.hlsli"
#include "Spectral-Tracing/Spectrum/ColorSpectrums.hlsli"
#include "Spectral-Tracing/SpectralValue/SpectralValue.hlsli"
#include "Path-Tracing/Fresnel.hlsli"
#include "Path-Tracing/Sellmeier.hlsli"
#include "Random.h"

#define CONSTANT_BOLTZMANN 1.38064852e-23f
#define CONSTANT_PLANK 6.62607015e-34f
#define CONSTANT_SPEED_OF_LIGHT 299792458.0f

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

[noinline] // Prevent inlining as it would otherwise explode compile time
float3 RoundTripTest(float3 lrgb)
{
    Spectrum s;

    Spectrum whiteD65 = WhiteSpectrum_D65();

    s.InitFromRGB(lrgb, eReflectance);
    s.Mul(whiteD65); // Reflectance -> Radiance

    return SpectrumToRGB(s);
}

//[noinline] // Prevent inlining as it would otherwise explode compile time
float3 RoundTripTest_v2(float3 lrgb, in SpectralContext ctx)
{
    SpectralValue s;
    s.FromRGB(lrgb, eIlluminant, ctx);
    return s.ToRGB(ctx);
}

float Luminance(Spectrum a)
{
    float lum = 0.0f;

    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
    {
        float lambda = IndexToLambda(i);
        float cieY = SampleCIE(lambda).y;
        lum += cieY * a.Samples[i];
    }

    return lum * SPECTRUM_DELTA_LAMBDA / cCIE_Y_integral;
}

Complex SampleIor_Lambda(float lambda, bool isGlass, bool isConductor)
{
    if (isGlass)
		return SampleIor_Glass(lambda);
	else if (isConductor)
		return SampleIor_Conductor(lambda);
	else
		return SampleIor_Dielectric(lambda);
}

Complex SampleIor(SpectralContext ctx, bool isGlass, bool isConductor)
{
#if defined(SPECTRAL_SINGLE_WAVELENGTH_SAMPLING)
    float lambda = ctx.Lambda;
#elif defined(SPECTRAL_HERO_SAMPLING)
    float lambda = ctx.GetHeroLambda();
#else
	float lambda = -1;
#endif

    return SampleIor_Lambda(lambda, isGlass, isConductor);
}

#if defined(SPECTRAL_HERO_SAMPLING)
HeroSpectrum ComputeHeroReflectWeights(float NdV, SpectralContext ctx, bool entering, bool isGlass, bool isConductor)
{
    HeroSpectrum s;
    s.Samples[0] = 1.0f;

    [unroll]
    for (int i = 1; i < NUM_HERO_SAMPLES; i++)
    {
		Complex iorMat = SampleIor_Lambda(ctx.GetLambda(i), isGlass, isConductor);
        Complex iorCurrent_i = entering ? IOR_AIR : iorMat;
        Complex iorNext_i = entering ? iorMat : IOR_AIR;

        float F_i = Fresnel_Maxwell(iorCurrent_i, iorNext_i, NdV, isConductor);
        s.Samples[i] = F_i;
    }
    return s;
}

HeroSpectrum ComputeHeroRefractWeights(float NdV, SpectralContext ctx, bool entering, bool isGlass, bool isConductor)
{
    HeroSpectrum s;
    s.Samples[0] = 1.0f;

    [unroll]
    for (int i = 1; i < NUM_HERO_SAMPLES; i++)
    {
		Complex iorMat = SampleIor_Lambda(ctx.GetLambda(i), isGlass, isConductor);
        Complex iorCurrent_i = entering ? IOR_AIR : iorMat;
        Complex iorNext_i = entering ? iorMat : IOR_AIR;

        float F_i = Fresnel_Maxwell(iorCurrent_i, iorNext_i, NdV, isConductor);
        Complex eta = Div(iorNext_i, iorCurrent_i);
        Complex eta2 = Mul(eta, eta);
        float eta2f = eta2.Re * eta2.Re + eta2.Im * eta2.Im;
        s.Samples[i] = eta2f * (1.0f - F_i);
    }
    return s;
}
#endif

#define RAYLEIGH_CONSTANT 5.8e-6 // Assuming IOR air is ~1

float RayleighSigma(float lambda)
{
    float lambda2 = lambda * lambda;
    float lambda4 = lambda2 * lambda2;
    lambda4 = lambda4 / 1000.0f / 1000.0f / 1000.0f; // nm -> um -> cm -> m

    return RAYLEIGH_CONSTANT / lambda4;
}

float RayleighPhase(float cosTheta)
{
    return (3.0f / (16.0f * PI)) * (1.0f + cosTheta * cosTheta);
}

float SampleRayleighCosTheta(float u)
{
    // Invert CDF approximation
    float z = 2.0f * u - 1.0f;

    float g = sqrt(1.0f + z * z);
    return z / g; // approximation
}

float SampleRayleighMediumT(inout RngInfo rngInfo, float lambda)
{
    float sigma_s = RayleighSigma(lambda);
    return -log(1.0f - PcgRand01(rngInfo.IndependentRngState)) / sigma_s;
}

float RayleighPhasePDF(float cosTheta)
{
    return 3.0f / (16.0f * PI) * (1.0f + cosTheta * cosTheta);
}

#endif