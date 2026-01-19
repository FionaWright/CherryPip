struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 binormal : TEXCOORD3;
};

#include "DualIncludes/Cbv.h"

Texture2D<float4> gDiffuse : register(t0);
Texture2D<float4> gNormal : register(t1);
SamplerState gSampler : register(s0);

ConstantBuffer<CbvRasterDebug> c_rasterDebug : register(b1);

float4 PSMain(VsOut input) : SV_TARGET
{
    float4 albedo = gDiffuse.Sample(gSampler, input.uv).rgba;
    float3 bumpSample = gNormal.Sample(gSampler, input.uv).rgb * 2.0f - 1.0f;

    float3 T = normalize(input.tangent);
    float3 B = normalize(input.binormal);
    float3 N = normalize(input.normal);
    float3 N_w = normalize(bumpSample.x * T + bumpSample.y * B + bumpSample.z * N);

    float NdL = dot(N_w, normalize(c_rasterDebug.DirLighting));

    switch (c_rasterDebug.Mode)
    {
    case ePosition:
        return float4(input.position.xyz / input.position.w, 1);
    case eNormals:
        return float4(N_w, 1);
    case eTangent:
        return float4(input.tangent, 1);
    case eBinormal:
        return float4(input.binormal, 1);
    case eUV:
        return float4(input.uv, 0, 1);
    case eDirLighting:
        return float4(NdL.xxx, 1);
    case eTex:
        return albedo;
    case eDirLightingTex:
        return albedo * float4(NdL.xxx, 1);
    }
    return float4(0, 1, 1, 1);
}