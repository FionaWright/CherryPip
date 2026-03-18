#ifndef H_SPECTRAL_VALUE_SPECTRUM_H
#define H_SPECTRAL_VALUE_SPECTRUM_H

#include "Spectral-Tracing/Spectrum/RgbToSpectrum2019.hlsli"
#include "Spectral-Tracing/Spectrum/SpectrumToRGB2019.hlsli"

struct SpectralValue
{
    void Init(Spectrum v);

#include "Spectral-Tracing/SpectralValue/ISpectralValue.hlsli"

    Spectrum Value;
};

SpectralValue CreateSpectralValue(Spectrum v)
{
    SpectralValue s;
    s.Value = v;
    return s;
}

SpectralValue CreateBlackSpectralValue()
{
    return CreateSpectralValue(BlackSpectrum());
}

SpectralValue CreateWhiteSpectralValue()
{
    return CreateSpectralValue(WhiteSpectrum_E());
}

void SpectralValue::Init(Spectrum v) { Value = v; }
void SpectralValue::InitBlack() { Value = BlackSpectrum(); }
void SpectralValue::InitWhite() { Value = WhiteSpectrum_E(); }

void SpectralValue::FromRGB(float3 rgb, SpectrumType type, SpectralContext _)
{
    Value.InitFromRGB(rgb, type);
}

void SpectralValue::AddRGB(float3 rgb, SpectrumType type, SpectralContext _)
{
    Spectrum s;
    s.InitFromRGB(rgb, type);
    Value.Add(s);
}

float3 SpectralValue::ToRGB(SpectralContext _)
{
    return SpectrumToRGB(Value);
}

void SpectralValue::Add(SpectralValue v) { Value.Add(v.Value); }
void SpectralValue::Add(float scalar) { Value.Add(scalar); }
void SpectralValue::Sub(SpectralValue v) { Value.Sub(v.Value); }
void SpectralValue::Sub(float scalar) { Value.Sub(scalar); }
void SpectralValue::Mul(SpectralValue v) { Value.Mul(v.Value); }
void SpectralValue::Mul(float scalar) { Value.Mul(scalar); }
void SpectralValue::Div(SpectralValue v) { Value.Div(v.Value); }
void SpectralValue::Div(float scalar) { Value.Div(scalar); }

void SpectralValue::Sqrt() { Value.Sqrt(); }
void SpectralValue::Clamp(float lo, float hi) { Value.Clamp(lo, hi); }
void SpectralValue::Normalize() { Value.Normalize(); }
void SpectralValue::Exp() { Value.Exp(); }

SpectralValue Add(SpectralValue a, SpectralValue b) { return CreateSpectralValue(Add(a.Value, b.Value)); }
SpectralValue Add(SpectralValue a, float scalar) { return CreateSpectralValue(Add(a.Value, scalar)); }
SpectralValue Sub(SpectralValue a, SpectralValue b) { return CreateSpectralValue(Sub(a.Value, b.Value)); }
SpectralValue Sub(SpectralValue a, float scalar) { return CreateSpectralValue(Sub(a.Value, scalar)); }
SpectralValue Sub(float scalar, SpectralValue a) { return CreateSpectralValue(Sub(scalar, a.Value)); }
SpectralValue Mul(SpectralValue a, SpectralValue b) { return CreateSpectralValue(Mul(a.Value, b.Value)); }
SpectralValue Mul(SpectralValue a, float scalar) { return CreateSpectralValue(Mul(a.Value, scalar)); }
SpectralValue Div(SpectralValue a, SpectralValue b) { return CreateSpectralValue(Div(a.Value, b.Value)); }
SpectralValue Div(SpectralValue a, float scalar) { return CreateSpectralValue(Div(a.Value, scalar)); }
SpectralValue Div(float scalar, SpectralValue a) { return CreateSpectralValue(Div(scalar, a.Value)); }

SpectralValue Clamp(SpectralValue a, float lo, float hi) { return CreateSpectralValue(Clamp(a.Value, lo, hi)); }
SpectralValue Sqrt(SpectralValue a) { return CreateSpectralValue(Sqrt(a.Value)); }
SpectralValue Normalize(SpectralValue a) { return CreateSpectralValue(Normalize(a.Value)); }
SpectralValue Exp(SpectralValue a) { return CreateSpectralValue(Exp(a.Value)); }

SpectralValue Lerp(SpectralValue a, SpectralValue b, float t) { return CreateSpectralValue(Lerp(a.Value, b.Value, t)); }
SpectralValue Lerp(SpectralValue a, float end, float t) { return CreateSpectralValue(Lerp(a.Value, end, t)); }
SpectralValue Lerp(float start, SpectralValue a, float t) { return CreateSpectralValue(Lerp(start, a.Value, t)); }

float Luminance(SpectralValue a, SpectralContext _) { return Luminance(a.Value); }

bool SpectralValue::IsBlack()
{
    [unroll]
    for (int i = 0; i < NUM_SPECTRUM_SAMPLES; i++)
        if (Value.Samples[i] > 0.0f)
            return false;
    return true;
}

#endif