#ifndef H_HERO_SAMPLE_H
#define H_HERO_SAMPLE_H

#define NUM_HERO_SAMPLES 3
#define HERO_DELTA_LAMBDA (VISIBLE_LIGHT_SPECTRUM_SIZE / float(NUM_HERO_SAMPLES-1)) 

struct HeroSpectrum
{
    void Init(float values[NUM_HERO_SAMPLES]);
    void InitBlack();
    void InitWhite();

    void ReflectanceRgbToSpectrum(float3 rgb, SpectralContext ctx);
    void IlluminantRgbToSpectrum(float3 rgb, SpectralContext ctx);

    void Add(HeroSpectrum spectrum);
    void Add(float scalar);
    void Sub(HeroSpectrum spectrum);
    void Sub(float scalar);
    void Mul(HeroSpectrum spectrum);
    void Mul(float scalar);
    void Div(HeroSpectrum spectrum);
    void Div(float scalar);

    float Samples[NUM_HERO_SAMPLES];
};

void HeroSpectrum::Init(float values[NUM_HERO_SAMPLES])
{
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        Samples[i] = values[i];
}

void HeroSpectrum::InitBlack()
{
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        Samples[i] = 0.0f;
}

void HeroSpectrum::InitWhite()
{
    [unroll]    
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        Samples[i] = 1.0f;
}

void HeroSpectrum::Add(HeroSpectrum spectrum)
{
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        Samples[i] += spectrum.Samples[i];
}

void HeroSpectrum::Add(float scalar)
{
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        Samples[i] += scalar;
}

void HeroSpectrum::Sub(HeroSpectrum spectrum)
{
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        Samples[i] -= spectrum.Samples[i];
}

void HeroSpectrum::Sub(float scalar)
{
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        Samples[i] -= scalar;
}

void HeroSpectrum::Mul(HeroSpectrum spectrum)
{
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        Samples[i] *= spectrum.Samples[i];
}

void HeroSpectrum::Mul(float scalar)
{
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        Samples[i] *= scalar;
}

void HeroSpectrum::Div(HeroSpectrum spectrum)
{
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        Samples[i] /= spectrum.Samples[i];
}

void HeroSpectrum::Div(float scalar)
{
    float inv = 1.0f / scalar;

    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        Samples[i] *= inv;
}

HeroSpectrum Add(HeroSpectrum a, HeroSpectrum b)
{
    HeroSpectrum r;
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        r.Samples[i] = a.Samples[i] + b.Samples[i];
    return r;
}

HeroSpectrum Add(HeroSpectrum a, float s)
{
    HeroSpectrum r;
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        r.Samples[i] = a.Samples[i] + s;
    return r;
}

HeroSpectrum Sub(HeroSpectrum a, HeroSpectrum b)
{
    HeroSpectrum r;
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        r.Samples[i] = a.Samples[i] - b.Samples[i];
    return r;
}

HeroSpectrum Sub(HeroSpectrum a, float s)
{
    HeroSpectrum r;
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        r.Samples[i] = a.Samples[i] - s;
    return r;
}

HeroSpectrum Mul(HeroSpectrum a, HeroSpectrum b)
{
    HeroSpectrum r;
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        r.Samples[i] = a.Samples[i] * b.Samples[i];
    return r;
}

HeroSpectrum Mul(HeroSpectrum a, float s)
{
    HeroSpectrum r;
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        r.Samples[i] = a.Samples[i] * s;
    return r;
}

HeroSpectrum Div(HeroSpectrum a, HeroSpectrum b)
{
    HeroSpectrum r;
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        r.Samples[i] = a.Samples[i] / b.Samples[i];
    return r;
}

HeroSpectrum Div(HeroSpectrum a, float s)
{
    HeroSpectrum r;
    float inv = 1.0f / s;

    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        r.Samples[i] = a.Samples[i] * inv;
    return r;
}

HeroSpectrum Clamp(HeroSpectrum a, float lo, float hi)
{
    HeroSpectrum r;
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        r.Samples[i] = clamp(a.Samples[i], lo, hi);
    return r;
}

HeroSpectrum Sqrt(HeroSpectrum a)
{
    HeroSpectrum r;
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        r.Samples[i] = sqrt(a.Samples[i]);
    return r;
}

HeroSpectrum Normalize(HeroSpectrum a)
{
    HeroSpectrum r;

    float sum = 0.0f;
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        sum += a.Samples[i];

    float inv = (sum > 0.0f) ? (1.0f / sum) : 0.0f;

    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        r.Samples[i] = a.Samples[i] * inv;

    return r;
}

HeroSpectrum Exp(HeroSpectrum a)
{
    HeroSpectrum r;
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        r.Samples[i] = exp(a.Samples[i]);
    return r;
}

HeroSpectrum Lerp(HeroSpectrum a, HeroSpectrum b, float t)
{
    HeroSpectrum r;
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        r.Samples[i] = lerp(a.Samples[i], b.Samples[i], t);
    return r;
}

HeroSpectrum Lerp(HeroSpectrum a, float end, float t)
{
    HeroSpectrum r;
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        r.Samples[i] = lerp(a.Samples[i], end, t);
    return r;
}

HeroSpectrum Lerp(float start, HeroSpectrum a, float t)
{
    HeroSpectrum r;
    [unroll]
    for (int i = 0; i < NUM_HERO_SAMPLES; i++)
        r.Samples[i] = lerp(start, a.Samples[i], t);
    return r;
}

#endif