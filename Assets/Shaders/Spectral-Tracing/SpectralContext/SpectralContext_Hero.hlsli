#ifndef H_SPECTRALCONTEXT_HERO_H
#define H_SPECTRALCONTEXT_HERO_H

#include "Spectral-Tracing/Spectrum/HeroSpectrum.hlsli"

struct SpectralContext
{
    void Init(float lambdas[NUM_HERO_SAMPLES]);
    float GetHeroLambda();
    float GetLambda(int i);

#include "Spectral-Tracing/SpectralContext/ISpectralContext.hlsli"

    float Lambdas[NUM_HERO_SAMPLES];
};

#include "Spectral-Tracing/SpectralUtils.hlsli"

void SpectralContext::Init(float lambdas[NUM_HERO_SAMPLES])
{
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        Lambdas[i] = lambdas[i];
}

void SpectralContext::Create(inout RngInfo rngInfo)
{
    float heroWavelength = SampleVisibleWavelength(rngInfo.IndependentRngState);
    Lambdas[0] = heroWavelength;

    float heroOffset = heroWavelength - VISIBLE_LIGHT_SPECTRUM_MIN;

    [unroll]
    for (int i = 1; i < NUM_HERO_SAMPLES; i++)
        Lambdas[i] = VISIBLE_LIGHT_SPECTRUM_MIN + fmod(heroOffset + (i * HERO_DELTA_LAMBDA), VISIBLE_LIGHT_SPECTRUM_SIZE);

    PDF = 1.0f / (float)VISIBLE_LIGHT_SPECTRUM_SIZE; // Only hero sample was random, others were deterministic
}

float SpectralContext::GetHeroLambda()
{
    return Lambdas[0];
}

float SpectralContext::GetLambda(int i)
{
    return Lambdas[i];
}

#endif