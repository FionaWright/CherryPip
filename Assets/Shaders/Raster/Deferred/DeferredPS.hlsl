#include "CBV.h"

struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

// Temp CBV
ConstantBuffer<CbvRasterDebug> cbv : register(b0);

Texture2D    gRgbaAlbedo  : register(t0);
Texture2D    gRgbNormal_ADepth  : register(t1);
SamplerState gSampler : register(s0);

float4 PSMain(VsOut input) : SV_Target
{
    float4 albedoSample = gRgbaAlbedo.Sample(gSampler, input.uv);
    float3 normalSample = gRgbNormal_ADepth.Sample(gSampler, input.uv).rgb * 2.0f - 1.0f;

    bool isSkybox = all(abs(normalSample+1) < 1e-4);
    if (isSkybox)
        return float4(albedoSample.rgb, 1.0f);

    normalSample = normalize(normalSample);

    float3 L = -cbv.DirLightDir;

    float NdL = saturate(dot(normalSample, L));
    float3 diffuse = albedoSample.rgb * NdL;
    return float4(diffuse, 0.0f);
}