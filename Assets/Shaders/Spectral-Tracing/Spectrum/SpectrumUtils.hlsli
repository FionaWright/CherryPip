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

#if defined(SPECTRAL_HERO_SAMPLING)
HeroSpectrum ComputeHeroReflectWeights(float NdV, SpectralContext ctx, bool entering, bool isGlass, bool isConductor)
{
    HeroSpectrum s;
    s.Samples[0] = 1.0f;

    Complex iorMat_0 = SampleIor_Lambda(ctx.GetHeroLambda(), isGlass, isConductor);
    Complex iorCurrent_0 = Ternary(entering, IOR_AIR, iorMat_0);
    Complex iorNext_0 = Ternary(entering, iorMat_0, IOR_AIR);
    float F_0 = Fresnel_Maxwell(iorCurrent_0, iorNext_0, NdV, isConductor);

    [unroll]
    for (int i = 1; i < NUM_HERO_SAMPLES; i++)
    {
		Complex iorMat_i = SampleIor_Lambda(ctx.GetLambda(i), isGlass, isConductor);
        Complex iorCurrent_i = Ternary(entering, IOR_AIR, iorMat_i);
        Complex iorNext_i = Ternary(entering, iorMat_i, IOR_AIR);

        Complex eta = Div(iorNext_i, iorCurrent_i);
        Complex eta2 = Mul(eta, eta);
        float eta2f = eta2.Re * eta2.Re + eta2.Im * eta2.Im;

        float F_i = Fresnel_Maxwell(iorCurrent_i, iorNext_i, NdV, isConductor);
        s.Samples[i] = F_i / max(1e-6, F_0);
    }
    return s;
}

HeroSpectrum ComputeHeroRefractWeights(float NdV, SpectralContext ctx, bool entering, bool isGlass, bool isConductor)
{
    HeroSpectrum s;
    s.Samples[0] = 1.0f;

    Complex iorMat_0 = SampleIor_Lambda(ctx.GetHeroLambda(), isGlass, isConductor);
    Complex iorCurrent_0 = Ternary(entering, IOR_AIR, iorMat_0);
    Complex iorNext_0 = Ternary(entering, iorMat_0, IOR_AIR);
    float F_0 = Fresnel_Maxwell(iorCurrent_0, iorNext_0, NdV, isConductor);

    Complex eta_0 = Div(iorNext_0, iorCurrent_0);
    Complex eta2_0 = Mul(eta_0, eta_0);
    float eta2f_0 = eta2_0.Re * eta2_0.Re + eta2_0.Im * eta2_0.Im;
    float weight = eta2f_0 * (1.0f - F_0);

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
        s.Samples[i] = eta2f * (1.0f - F_i) / max(1e-6, weight);
    }
    return s;
}
#endif

#endif