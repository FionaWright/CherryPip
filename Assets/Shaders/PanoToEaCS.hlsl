#include "HlslMath.h"

Texture2D<float3> gPano : register(t0);
RWTexture2D<float4> gEA : register(u0);

SamplerState gSampler : register(s0);

cbuffer CB : register(b0)
{
    uint gOutputWidth;
    uint gOutputHeight;
    uint gInputWidth;
    uint gInputHeight;

    float gRotation;
    float3 p;
}

[numthreads(16,16,1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= gOutputWidth || dispatchThreadID.y >= gOutputHeight)
        return;

    // Normalized coordinates in [0,1]
    float u = (dispatchThreadID.x + 0.5f) / gOutputWidth;
    float v = (dispatchThreadID.y + 0.5f) / gOutputHeight;

    float3 dir = EaSquareToSphere(float2(u, v));
    float2 panoUV = PanoSphereToSquare(dir);
    panoUV.x = frac(panoUV.x + gRotation);
    panoUV.y = 1 - panoUV.y;

    float3 color = gPano.SampleLevel(gSampler, panoUV, 0.0f).rgb;

    gEA[dispatchThreadID.xy] = float4(color, 1);
}
