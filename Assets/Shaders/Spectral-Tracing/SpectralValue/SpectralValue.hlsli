#ifndef H_SPECTRAL_VALUE_H
#define H_SPECTRAL_VALUE_H

struct SpectralValue;

SpectralValue CreateBlackSpectralValue();
SpectralValue CreateWhiteSpectralValue();

SpectralValue Add(SpectralValue a, SpectralValue b);
SpectralValue Add(SpectralValue a, float scalar);
SpectralValue Sub(SpectralValue a, SpectralValue b);
SpectralValue Sub(SpectralValue a, float scalar);
SpectralValue Sub(float scalar, SpectralValue a);
SpectralValue Mul(SpectralValue a, SpectralValue b);
SpectralValue Mul(SpectralValue a, float scalar);
SpectralValue Div(SpectralValue a, SpectralValue b);
SpectralValue Div(SpectralValue a, float scalar);
SpectralValue Div(float scalar, SpectralValue a);

SpectralValue Clamp(SpectralValue a, float lo, float hi);
SpectralValue Sqrt(SpectralValue a);
SpectralValue Normalize(SpectralValue a);
SpectralValue Exp(SpectralValue a);

SpectralValue Lerp(SpectralValue a, SpectralValue b, float t);
SpectralValue Lerp(SpectralValue a, float end, float t);
SpectralValue Lerp(float start, SpectralValue a, float t);

float Luminance(SpectralValue a, SpectralContext ctx);

#ifdef SPECTRAL_SINGLE_WAVELENGTH_SAMPLING
#    include "Spectral-Tracing/SpectralValue/SpectralValue_Float.hlsli"
#else
#    include "Spectral-Tracing/SpectralValue/SpectralValue_Spectrum.hlsli"
#endif

#endif