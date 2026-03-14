#ifndef H_SPECTRAL_VALUE_H
#define H_SPECTRAL_VALUE_H

struct SpectralValue;

SpectralValue CreateBlackSpectralValue();
SpectralValue CreateWhiteSpectralValue();

SpectralValue Add(SpectralValue a, SpectralValue b);
SpectralValue Sub(SpectralValue a, SpectralValue b);
SpectralValue Mul(SpectralValue a, SpectralValue b);
SpectralValue Mul(SpectralValue a, float scalar);
SpectralValue Div(SpectralValue a, SpectralValue b);
SpectralValue Div(SpectralValue a, float scalar);
SpectralValue Clamp(SpectralValue a, float lo, float hi);
SpectralValue Sqrt(SpectralValue a);
SpectralValue Normalize(SpectralValue a);
SpectralValue Exp(SpectralValue a);

#ifdef SINGLE_LAMBDA_RENDERING
#    include "Spectral-Tracing/SpectralValue/SpectralValue_Float.hlsli"
#else
#    include "Spectral-Tracing/SpectralValue/SpectralValue_Spectrum.hlsli"
#endif

#endif