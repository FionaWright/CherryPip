#include "CBV.h"

struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

ConstantBuffer<CbvDeferredLighting> cbv : register(b0);

Texture2D    gRgbaAlbedo  : register(t0);
Texture2D    gRgbNormal_ADepth  : register(t1);
SamplerState gSampler : register(s0);

float3 ReconstructViewDir(float2 uv)
{
    float4 clip;
    clip.xy = uv * float2(2, -2) + float2(-1, 1);
    clip.z  = 1.0f;
    clip.w  = 1.0f;

    float4 view = mul(cbv.InvP, clip);
    return normalize(view.xyz / view.w);
}

float4 PSMain(VsOut input) : SV_Target
{
    float4 albedoSample = gRgbaAlbedo.Sample(gSampler, input.uv);
    float4 normalDepthSample = gRgbNormal_ADepth.Sample(gSampler, input.uv);
    float3 normalSample = normalDepthSample.rgb * 2.0f - 1.0f;
    float depthSample = normalDepthSample.a;

    bool isSkybox = all(abs(normalSample+1) < 1e-4);
    if (isSkybox)
        return float4(albedoSample.rgb, 1.0f);

    float3 N = normalize(normalSample);
    float3 L = -cbv.DirLightDir;
    float3 V = ReconstructViewDir(input.uv);

    float NdL = saturate(dot(N, L));

    float3 diffuse = albedoSample.rgb * NdL;
    return float4(diffuse, 0.0f);
}