#include "CBV.h"
#include "Microfacet.hlsli"

struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

ConstantBuffer<CbvDeferredLighting> c_deferredLighting : register(b0);
ConstantBuffer<CbvRasterDebug> c_rasterDebug : register(b1);

Texture2D<float4>    gRgbaAlbedo  : register(t0);
Texture2D<float4>    gRgbNormal_ADepth  : register(t1);
Texture2D<float4> gRoughMet : register(t2);
Texture2D<float2> gBrdfInt : register(t3);
TextureCube gEnvMap : register(t4);
TextureCube gIrradiance : register(t5);

SamplerState gSampler : register(s0);

float3 ReconstructViewDir(float2 uv)
{
    float4 clip;
    clip.xy = uv * float2(2, -2) + float2(-1, 1);
    clip.z  = 1.0f;
    clip.w  = 1.0f;

    float4 view = mul(c_deferredLighting.InvP, clip);
	view.y = -view.y;
    return normalize(view.xyz / view.w);
}

float4 PSMain(VsOut input) : SV_Target
{
    float4 albedoSample = gRgbaAlbedo.Sample(gSampler, input.uv);
	float4 albedoGamma = float4(pow(albedoSample.rgb, 2.2f), albedoSample.a);

    float4 normalDepthSample = gRgbNormal_ADepth.Sample(gSampler, input.uv);
    float3 normalSample = normalDepthSample.rgb * 2.0f - 1.0f;
    float depthSample = normalDepthSample.a;

    bool isSkybox = all(abs(normalSample+1) < 1e-4);
    if (isSkybox)
        return float4(albedoSample.rgb, 1.0f);

	float2 roughMet = gRoughMet.Sample(gSampler, input.uv).rg;
	float3 emission = float3(0,0,0); // TODO?

    float3 N = normalize(normalSample);
    float3 L = -c_deferredLighting.DirLightDir;
    float3 V = ReconstructViewDir(input.uv);
	float3 H = normalize(L + V);

    float NdL = saturate(dot(N, L));
    float NdV = saturate(dot(N, V));
    float NdH = saturate(dot(N, H));
    float HdV = saturate(dot(H, V));

	float3 irradianceIblSample = gIrradiance.SampleLevel(gSampler, N, 0).rgb;

	float roughness = roughMet.r;
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

    float3 combinedBrdf = diffuseBrdf + specularBrdf;
    float3 Lo = pow(combinedBrdf, 1.0f / 2.2f);

    float3 R = reflect(-V, N);
    float lod = roughness * (c_deferredLighting.MaxCubemapMipMaps - 1);
    float3 envSample = gEnvMap.SampleLevel(gSampler, R, lod).rgb;
    //float3 envSample = gEnvMap.SampleLevel(gSampler, R, 8).rgb;

    float2 brdfIntSample = gBrdfInt.SampleLevel(gSampler, saturate(float2(max(NdV, 0), roughness)), 0).rg;
    float3 indirectSpecular = envSample * (F * brdfIntSample.r + brdfIntSample.g) * kS;
    indirectSpecular = max(0, indirectSpecular);
    float3 combinedBrdfWithIndirect = combinedBrdf + indirectSpecular;
    float3 LoWithIndirect = pow(combinedBrdfWithIndirect, 1.0f / 2.2f);

    switch (c_rasterDebug.Mode)
    {
    case ePosition:
        return float4(input.position.xyz / input.position.w, 1);
	case eNormalsVertex:
		return float4(1, 0, 0, 1); // Does not exist
    case eNormalsBumped:
        return float4(N, 1);
    case eTangent:
        return float4(1, 0, 0, 1); // Does not exist
    case eBinormal:
        return float4(1, 0, 0, 1); // Does not exist
    case eUV:
        return float4(input.uv, 0, 1);
    case eDirLighting:
        return float4(NdL.xxx, 1);
    case eTex:
        return albedoSample;
    case eDirLightingTex:
        return albedoSample * float4(NdL.xxx, 1);
    case eRoughness:
        return float4(roughness.xxx, 1);
    case eMetalness:
        return float4(metalness.xxx, 1);
    case eEmission:
        return float4(emission, 1);
    case eViewDir:
        return float4(V, 1);
	case eFresnel:
		return float4(F, 1);
    case eReflection:
        return float4(envSample, 1);
    case eIrradianceIBL:
        return float4(irradianceIblSample, 1);
    case eMicrofacetSpecular:
        return float4(pow(specularBrdf, 1.0f / 2.2f), 1);
    case eMicrofacetDiffuse:
        return float4(pow(diffuseBrdf, 1.0f / 2.2f), 1);
    case eMicrofacetIndirect:
        return float4(pow(indirectSpecular, 1.0f / 2.2f), 1);
    case eMicrofacetLo:
        return float4(Lo, 1);
    case eMicrofacetLoWithIndirect:
        return float4(LoWithIndirect, 1);
    }
    return float4(0, 1, 1, 1);
}