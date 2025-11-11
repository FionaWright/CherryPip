struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 binormal : TEXCOORD3;
};

#include "DualIncludes/Cbv.h"

ConstantBuffer<CbvRasterDebug> c_rasterDebug : register(b1);
Texture2D<float4> gDiffuse : register(t0);
SamplerState gSampler : register(s0);

float4 PSMain(VsOut input) : SV_TARGET
{
    float NdL = dot(input.normal, normalize(c_rasterDebug.DirLighting));
    float4 albedo = gDiffuse.Sample(gSampler, input.uv).rgba;

    switch (c_rasterDebug.Mode)
    {
    case ePosition:
        return float4(input.position.xyz / input.position.w, 1);
    case eNormals:
        return float4(input.normal, 1);
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