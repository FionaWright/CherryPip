#ifndef H_SPECTRALCONTEXT_HERO_H
#define H_SPECTRALCONTEXT_HERO_H

#include "Spectral-Tracing/SpectralValue/HeroSample.hlsli"

struct SpectralContext
{
    void Init(float lambdas[NUM_HERO_SAMPLES]);

#include "Spectral-Tracing/SpectralContext/ISpectralContext.hlsli"

    float HeroLambda;
    float NonHeroLambdas[NUM_HERO_SAMPLES-1];
};

void SpectralContext::Init(float lambdas[NUM_HERO_SAMPLES])
{
    HeroLambda = lambdas[0];
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES-1; i++)
        NonHeroLambdas[i] = lambdas[i+1];
}

void SpectralContext::Create(inout RngInfo rngInfo)
{
    // TODO
}

#endif