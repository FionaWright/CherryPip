#ifndef H_SPECTRALCONTEXT_FLOAT_H
#define H_SPECTRALCONTEXT_FLOAT_H

struct SpectralContext
{
    void Init(float lambda);

#include "Spectral-Tracing/SpectralContext/ISpectralContext.hlsli"

    float Lambda;
};

#include "Spectral-Tracing/SpectralUtils.hlsli"

void SpectralContext::Init(float lambda)
{
    Lambda = lambda;
}

void SpectralContext::Create(inout RngInfo rngInfo)
{
    Lambda = SampleVisibleWavelength(rngInfo.IndependentRngState);
    PDF = 1.0f / (float)VISIBLE_LIGHT_SPECTRUM_SIZE; // Assuming uniform sampling
}

#endif