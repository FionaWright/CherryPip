#ifndef H_SPECTRUM_UTILS_H
#define H_SPECTRUM_UTILS_H

float LambdaToIndex(float lambda);
float IndexToLambda(float idx);

#include "Spectral-Tracing/Spectrum/Spectrum.hlsli"
#include "Spectral-Tracing/SpectralData/CIE2006.hlsli"
#include "Spectral-Tracing/Spectrum/SpectrumToRGB2019.hlsli"
#include "Spectral-Tracing/SampleIOR.hlsli"

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

#if defined(SPECTRAL_HERO_SAMPLING)
HeroSpectrum ComputeHeroReflectWeights(float NdV, SpectralContext ctx, bool entering, bool isGlass, bool isConductor)
{
    HeroSpectrum s;
    s.Samples[0] = 1.0f;

    [unroll]
    for (int i = 1; i < NUM_HERO_SAMPLES; i++)
    {
		Complex iorMat = SampleIor_Lambda(ctx.GetLambda(i), isGlass, isConductor);
        Complex iorCurrent_i = Ternary(entering, IOR_AIR, iorMat);
        Complex iorNext_i = Ternary(entering, iorMat, IOR_AIR);

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
        Complex iorCurrent_i = Ternary(entering, IOR_AIR, iorMat);
        Complex iorNext_i = Ternary(entering, iorMat, IOR_AIR);

        float F_i = Fresnel_Maxwell(iorCurrent_i, iorNext_i, NdV, isConductor);
        Complex eta = Div(iorNext_i, iorCurrent_i);
        Complex eta2 = Mul(eta, eta);
        float eta2f = eta2.Re * eta2.Re + eta2.Im * eta2.Im;
        s.Samples[i] = eta2f * (1.0f - F_i);
    }
    return s;
}
#endif

#endif