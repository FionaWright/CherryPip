#ifndef H_SPECTRAL_VALUE_HERO_H
#define H_SPECTRAL_VALUE_HERO_H

#include "Spectral-Tracing/Spectrum/RgbToSpectrum2019.hlsli"
#include "Spectral-Tracing/Spectrum/SpectrumToRGB2019.hlsli"
#include "Spectral-Tracing/SpectralValue/HeroSpectrum.hlsli"

struct SpectralValue
{
    void Init(HeroSpectrum value);

#include "Spectral-Tracing/SpectralValue/ISpectralValue.hlsli"

    HeroSpectrum Value;
};

SpectralValue CreateSpectralValue(HeroSpectrum v)
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

void SpectralValue::Init(HeroSpectrum v) { Value = v; }
void SpectralValue::InitBlack() { Value.InitBlack(); }
void SpectralValue::InitWhite() { Value.InitWhite(); }

void SpectralValue::FromRGB(float3 rgb, SpectrumType type, SpectralContext ctx)
{
    Value.InitBlack();

    if (type == eReflectance)
        Value.ReflectanceRgbToSpectrum(rgb, ctx);
    else if (type == eIlluminant)
        Value.IlluminantRgbToSpectrum(rgb, ctx);
}

void SpectralValue::AddRGB(float3 rgb, SpectrumType type, SpectralContext ctx)
{
    SpectralValue s2;
    s2.FromRGB(rgb, type, ctx);
    Value.Add(s2.Value);
}

float3 SpectralValue::ToRGB(SpectralContext ctx)
{
    return HeroToRGB(Value, ctx);
}

void SpectralValue::Add(SpectralValue v)
{
    Value.Add(v.Value);
}

void SpectralValue::Add(float scalar)
{
    Value.Add(scalar);
}

void SpectralValue::Sub(SpectralValue v)
{
    Value.Sub(v.Value);
}

void SpectralValue::Sub(float scalar)
{
    Value.Sub(scalar);
}

void SpectralValue::Mul(SpectralValue v)
{
    Value.Mul(v.Value);
}

void SpectralValue::Mul(float scalar)
{
    Value.Mul(scalar);
}

void SpectralValue::Div(SpectralValue v)
{
    Value.Div(v.Value);
}

void SpectralValue::Div(float scalar)
{
    Valu.Div(scalar);
}

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

void SpectralValue::Sqrt()
{
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        Value.Samples[i] = sqrt(Value.Samples[i]);
}

void SpectralValue::Clamp(float lo, float hi)
{
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        Value.Samples[i] = clamp(Value.Samples[i], lo, hi);
}

void SpectralValue::Normalize()
{
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        Value.Samples[i] = saturate(Value.Samples[i]);
}

void SpectralValue::Exp()
{
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        Value.Samples[i] = exp(Value.Samples[i]);
}

float Luminance(SpectralValue a, SpectralContext ctx)
{
    return -1;
}

bool SpectralValue::IsBlack()
{
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        if (Value.Samples[i] > 0.0f)
            return false;
    return true;
}

#endif