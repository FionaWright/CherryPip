#include "DualIncludes/HlslMath.h"

Texture2D<float3> gPano : register(t0);
RWTexture2DArray<float4> gCubemap : register(u0);

SamplerState gSampler : register(s0);

cbuffer CB : register(b0)
{
    uint gOutputWidth; // Width=Height
    uint gInputWidth;
    uint gInputHeight;
    float gRotation;
}

[numthreads(16,16,1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x >= gOutputWidth || DTid.y >= gOutputWidth)
        return;

    uint face = DTid.z;
    float2 uv = (float2(DTid.xy) / float2(gOutputWidth.xx - 1));
    uv = uv * 2.0f - 1.0f; // [-1, 1]

    float3 dir = CubemapCubeToSphere(face, uv);
    float2 panoUV = PanoSphereToSquare(dir);
    panoUV.x = frac(panoUV.x + gRotation);
    panoUV.y = 1 - panoUV.y;

    float3 color = gPano.SampleLevel(gSampler, panoUV, 0.0f).rgb;

    gCubemap[DTid] = float4(color, 1);
}
