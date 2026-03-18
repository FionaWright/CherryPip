#ifndef H_SPECTRALCONTEXT_HERO_H
#define H_SPECTRALCONTEXT_HERO_H

#include "Spectral-Tracing/SpectralValue/HeroSpectrum.hlsli"

struct SpectralContext
{
    void Init(float lambdas[NUM_HERO_SAMPLES]);
    void GetHeroLambda();
    void GetLambda(int i);

#include "Spectral-Tracing/SpectralContext/ISpectralContext.hlsli"

    float Lambdas[NUM_HERO_SAMPLES];
};

void SpectralContext::Init(float lambdas[NUM_HERO_SAMPLES])
{
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        Lambdas[i] = lambdas[i];
}

void SpectralContext::Create(inout RngInfo rngInfo)
{
    // TODO
}

void SpectralContext::GetHeroLambda()
{
    return Lambdas[0];
}

void SpectralContext::GetLambda(int i)
{
    return Lambdas[i];
}

#endif