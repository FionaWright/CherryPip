#include "CBV.h"

struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

ConstantBuffer<CbvFilterBoxAndGauss> c_box : register(b0);

Texture2D    gTex  : register(t0);
SamplerState gSampler : register(s0);

// Temp:
#include "DebugWindow.hlsli"
#include "Path-Tracing/Spectral-Tracing/Spectrum.hlsli"
#include "Path-Tracing/Spectral-Tracing/ColorSpectrums.hlsli"
#include "Path-Tracing/Spectral-Tracing/SpectralUtils.hlsli"
#include "Path-Tracing/Spectral-Tracing/RgbToSpectrum.hlsli"
#include "Path-Tracing/Spectral-Tracing/SpectrumToRGB.hlsli"

float4 PSMain(VsOut input) : SV_Target
{
#include "Path-Tracing/Spectral-Tracing/Tests.hlsli"

    float3 sum = 0.0;
    int kernelSize = (c_box.Radius * 2 + 1);
    int sampleCount = kernelSize * kernelSize;

    for (float y = -c_box.Radius; y <= c_box.Radius; ++y)
    {
        for (float x = -c_box.Radius; x <= c_box.Radius; ++x)
        {
            float2 offset = float2(x, y) * c_box.TexelSize;
            sum += gTex.Sample(gSampler, input.uv + offset).rgb;
        }
    }

    return float4(sum / sampleCount, 1);
}