#include "DualIncludes/Cbv.h"
#include "Microfacet.hlsli"

struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float3 binormal : TEXCOORD3;
    float3 viewDir : TEXCOORD4;
};

Texture2D<float4> gDiffuse : register(t0);
Texture2D<float4> gNormal : register(t1);
Texture2D<float4> gRoughnessMetallic : register(t2);
Texture2D<float4> gEmissive : register(t3);
TextureCube gIrradiance : register(t4);
SamplerState gSampler : register(s0);

ConstantBuffer<CbvRasterDebug> c_rasterDebug : register(b2);

// Missing from Alkali:
// Indirect env map reflections
// Thin film interference
// Shadows
// Alpha Test

float4 PSMain(VsOut input) : SV_TARGET
{
    float4 albedo = gDiffuse.Sample(gSampler, input.uv).rgba;
	float4 albedoGamma = float4(pow(albedo.rgb, 2.2f), albedo.a);

    float3 bumpSample = gNormal.Sample(gSampler, input.uv).rgb * 2.0f - 1.0f;
    bumpSample.y = -bumpSample.y; // DX convention

    float2 roughMet = gRoughnessMetallic.Sample(gSampler, input.uv).gb;
    float3 emission = gEmissive.Sample(gSampler, input.uv).rgb;

    float3 T = normalize(input.tangent);
    float3 B = normalize(input.binormal);
    float3 N = normalize(input.normal);
    float3 N_w = normalize(bumpSample.x * T + bumpSample.y * B + bumpSample.z * N);

    float3 irradianceIblSample = gIrradiance.Sample(gSampler, N_w).rgb;

    float3 L = normalize(-c_rasterDebug.DirLightDir); // Surface to Light Vector
    float3 H = normalize(L + input.viewDir);

    float NdL = saturate(dot(N_w, L));
    float NdV = saturate(dot(N_w, input.viewDir));
    float NdH = saturate(dot(N_w, H));
    float HdV = saturate(dot(H, input.viewDir));

    float roughness = roughMet.r; // TODO: MaterialData CBV
    float metalness = roughMet.g;
    float3 F0 = lerp(0.04f, albedoGamma.rgb, metalness);

    float D = D_GGX(NdH, roughness);
    float3 F = F_Schlick(HdV, F0);
    float G = G_SmithFast(NdL, NdV, roughness);

    float3 kS = max(0.0f, F);
    float3 kD = 1.0f - kS;

    float3 specularBrdf = (D * F * G) / max(0.001f, 4.0f * NdV);
    // specularBrdf *= dirLightColor;
    specularBrdf *= irradianceIblSample;
    specularBrdf = max(0.0f, specularBrdf);

    float3 diffuseBrdf = kD * albedoGamma.rgb * NdL;
	// diffuseBrdf /= PI; // ?
    // diffuseBrdf *= dirLightColor;
    diffuseBrdf *= irradianceIblSample;
    diffuseBrdf *= 1.0f - metalness;

    float3 Lo = diffuseBrdf + specularBrdf;
    Lo = pow(Lo, 1.0f / 2.2f);

    switch (c_rasterDebug.Mode)
    {
    case ePosition:
        return float4(input.position.xyz / input.position.w, 1);
	case eNormalsVertex:
		return float4(input.normal, 1);
    case eNormalsBumped:
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
    case eRoughness:
        return float4(roughMet.rrr, 1);
    case eMetalness:
        return float4(roughMet.ggg, 1);
    case eEmission:
        return float4(emission, 1);
    case eViewDir:
        return float4(input.viewDir, 1);
	case eFresnel:
		return float4(F, 1);
    case eIrradianceIBL:
        return float4(irradianceIblSample, 1);
    case eMicrofacetSpecular:
        return float4(pow(specularBrdf, 1.0f / 2.2f), 1);
    case eMicrofacetDiffuse:
        return float4(pow(diffuseBrdf, 1.0f / 2.2f), 1);
    case eMicrofacetLo:
        return float4(Lo, 1);
    }
    return float4(0, 1, 1, 1);
}