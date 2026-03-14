#ifndef H_SPECTRAL_VALUE_FLOAT_H
#define H_SPECTRAL_VALUE_FLOAT_H

#include "Spectral-Tracing/Spectrum/RgbToSpectrum2019.hlsli"
#include "Spectral-Tracing/Spectrum/SpectrumToRGB2019.hlsli"

struct SpectralValue
{
    void Init(float v);

#include "Spectral-Tracing/SpectralValue/ISpectralValue.hlsli"

    float Value;
};

SpectralValue CreateSpectralValue(float v)
{
    SpectralValue s;
    s.Value = v;
    return s;
}

SpectralValue CreateBlackSpectralValue()
{
    SpectralValue s;
    s.InitBlack();
    return s;
}

SpectralValue CreateWhiteSpectralValue()
{
    SpectralValue s;
    s.InitWhite();
    return s;
}

void SpectralValue::Init(float v) { Value = v; }
void SpectralValue::InitBlack() { Value = 0.0f; }
void SpectralValue::InitWhite() { Value = 1.0f; }

void SpectralValue::FromRGB(float3 rgb, SpectrumType type, float lambda)
{
    Value = RgbToSpectrumSample(rgb, type, lambda);
}

void SpectralValue::AddRGB(float3 rgb, SpectrumType type, float lambda)
{
    Value += RgbToSpectrumSample(rgb, type, lambda);
}

float3 SpectralValue::ToRGB(float lambda)
{
    float pdf = 1.0f / (float)VISIBLE_LIGHT_SPECTRUM_SIZE; // Assuming uniform sampling
    return SpectrumSampleToRGB(Value, lambda, pdf);
}

void SpectralValue::Add(SpectralValue v)
{
    Value += v.Value;
}

void SpectralValue::Add(float scalar)
{
    Value += scalar;
}

void SpectralValue::Sub(SpectralValue v)
{
    Value -= v.Value;
}

void SpectralValue::Sub(float scalar)
{
    Value -= scalar;
}

void SpectralValue::Mul(SpectralValue v)
{
    Value *= v.Value;
}

void SpectralValue::Mul(float scalar)
{
    Value *= scalar;
}

void SpectralValue::Div(SpectralValue v)
{
    Value /= v.Value;
}

void SpectralValue::Div(float scalar)
{
    Value /= scalar;
}

void SpectralValue::Sqrt()
{
    Value = sqrt(Value);
}

void SpectralValue::Clamp(float lo, float hi)
{
    Value = clamp(Value, lo, hi);
}

void SpectralValue::Normalize()
{
    Value = saturate(Value); // clamps between 0 and 1
}

void SpectralValue::Exp()
{
    Value = exp(Value);
}

SpectralValue Add(SpectralValue a, SpectralValue b)
{
    SpectralValue r;
    r.Value = a.Value + b.Value;
    return r;
}

SpectralValue Add(SpectralValue a, float scalar)
{
    SpectralValue r;
    r.Value = a.Value + scalar;
    return r;
}

SpectralValue Sub(SpectralValue a, SpectralValue b)
{
    SpectralValue r;
    r.Value = a.Value - b.Value;
    return r;
}

SpectralValue Sub(SpectralValue a, float scalar)
{
    SpectralValue r;
    r.Value = a.Value - scalar;
    return r;
}

SpectralValue Sub(float scalar, SpectralValue a)
{
    SpectralValue r;
    r.Value = scalar - a.Value;
    return r;
}

SpectralValue Mul(SpectralValue a, SpectralValue b)
{
    SpectralValue r;
    r.Value = a.Value * b.Value;
    return r;
}

SpectralValue Mul(SpectralValue a, float scalar)
{
    SpectralValue r;
    r.Value = a.Value * scalar;
    return r;
}

SpectralValue Div(SpectralValue a, SpectralValue b)
{
    SpectralValue r;
    r.Value = a.Value / b.Value;
    return r;
}

SpectralValue Div(SpectralValue a, float scalar)
{
    SpectralValue r;
    r.Value = a.Value / scalar;
    return r;
}

SpectralValue Div(float scalar, SpectralValue a)
{
    SpectralValue r;
    r.Value = scalar / a.Value;
    return r;
}

SpectralValue Clamp(SpectralValue a, float lo, float hi)
{
    SpectralValue r;
    r.Value = clamp(a.Value, lo, hi);
    return r;
}

SpectralValue Sqrt(SpectralValue a)
{
    SpectralValue r;
    r.Value = sqrt(a.Value);
    return r;
}

SpectralValue Normalize(SpectralValue a)
{
    SpectralValue r;
    r.Value = saturate(a.Value);
    return r;
}

SpectralValue Exp(SpectralValue a)
{
    SpectralValue r;
    r.Value = exp(a.Value);
    return r;
}

SpectralValue Lerp(SpectralValue a, SpectralValue b, float t) { return CreateSpectralValue(lerp(a.Value, b.Value, t)); }
SpectralValue Lerp(SpectralValue a, float end, float t) { return CreateSpectralValue(lerp(a.Value, end, t)); }
SpectralValue Lerp(float start, SpectralValue a, float t) { return CreateSpectralValue(lerp(start, a.Value, t)); }

float Luminance(SpectralValue a, float lambda)
{
    float pdf = 1.0f / (float)VISIBLE_LIGHT_SPECTRUM_SIZE; // Assuming uniform sampling
    float cieY = SampleCIE(lambda).y;
    return a.Value * cieY / pdf;
}

#endif