#include "CBV.h"

struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

ConstantBuffer<CbvFilterBoxAndGauss> c_box : register(b0);

Texture2D    gTex  : register(t0);
SamplerState gSampler : register(s0);

#define MAX_RADIUS 7

float Gaussian(float x, float sigma)
{
    return exp(-(x*x) / (2.0 * sigma * sigma));
}

// Obvious optimization here to hardcode the weights or pass them in through CBV but I'm not too bothered as this filter isn't important (yet)

float4 PSMain(VsOut input) : SV_Target
{
    float weights[MAX_RADIUS + 1];

    float sigma = c_box.Radius / 3.0f;
    float weightSum = 0.0f;
    for (int i = 0; i <= c_box.Radius; i++)
    {
        weights[i] = Gaussian(i, sigma);
        weightSum += (i == 0) ? weights[i] : weights[i] * 2.0f; // *2 due to +x and -x
    }

    float3 sum = 0.0;

    for (float x = -c_box.Radius; x <= c_box.Radius; ++x)
    {
#ifdef HORIZONTAL
        float2 offset = float2(x, 0) * c_box.TexelSize;
#else
        float2 offset = float2(0, x) * c_box.TexelSize;
#endif
        float3 sample = gTex.Sample(gSampler, input.uv + offset).rgb;
        sum += sample * weights[abs(x)];
    }

    return float4(sum / weightSum, 1);
}