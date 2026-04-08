#ifndef H_WAVELENGTH_H
#define H_WAVELENGTH_H

struct SpectralContext;

#include "Random.h"

#if defined(SPECTRAL_SINGLE_WAVELENGTH_SAMPLING)
#    include "Spectral-Tracing/SpectralContext/SpectralContext_Float.hlsli"
#elif defined(SPECTRAL_HERO_SAMPLING)
#    include "Spectral-Tracing/SpectralContext/SpectralContext_Hero.hlsli"
#else
#    include "Spectral-Tracing/SpectralContext/SpectralContext_Spectrum.hlsli"
#endif

#endif