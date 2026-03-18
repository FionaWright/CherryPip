#ifndef H_SPECTRALCONTEXT_SPECTRUM_H
#define H_SPECTRALCONTEXT_SPECTRUM_H

// SpectralContext contains nothing

struct SpectralContext
{
#include "Spectral-Tracing/SpectralContext/ISpectralContext.hlsli"
};

void SpectralContext::Create(inout RngInfo rngInfo)
{
    PDF = -1;
}

#endif