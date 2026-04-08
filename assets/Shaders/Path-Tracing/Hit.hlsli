#ifndef H_HITPS_H
#define H_HITPS_H

#include "DebugPalette.hlsli"
#include "Random.h"
#include "MathUtils.hlsli"
#include "ShadingFrame.hlsli"

void Hit(inout RayQuery<RAY_FLAGS> q,
        out float4 albedo,
        out float3 Ng,
        out float3 Ns,
        out float3 Li,
        out PtMaterialData mat,
        out float2 uv,
        out float3 anisoDirAndStrength)
{
    PtInstanceData instance = gInstances[q.CommittedInstanceIndex()];
    mat = gMaterials[instance.MaterialIdx];

    uint3 tri = gIndexMegaBuffer[instance.IndexBufferOffset + q.CommittedPrimitiveIndex()];
    Vertex v0 = gVertexMegaBuffer[instance.VertexBufferOffset + tri.x];
    Vertex v1 = gVertexMegaBuffer[instance.VertexBufferOffset + tri.y];
    Vertex v2 = gVertexMegaBuffer[instance.VertexBufferOffset + tri.z];

    float2 barycentrics = q.CommittedTriangleBarycentrics();
    precise float3 bary = float3(1 - barycentrics.x - barycentrics.y, barycentrics.x, barycentrics.y);

    uv = v0.uv * bary.x + v1.uv * bary.y + v2.uv * bary.z;

	float3 p0 = mul(instance.M, float4(v0.position,1)).xyz;
	float3 p1 = mul(instance.M, float4(v1.position,1)).xyz;
	float3 p2 = mul(instance.M, float4(v2.position,1)).xyz;
	Ng = normalize( cross(p1 - p0, p2 - p0) );
    Ng = q.CommittedTriangleFrontFace() == 0 ? -Ng : Ng;

    Ns = v0.normal * bary.x + v1.normal * bary.y + v2.normal * bary.z;
    Ns = normalize(mul((float3x3)instance.MTI, Ns));
    if (cNormalMapsEnabled)
    {
        float3 bumpSample = gTextures[mat.TexIdxNormal].Sample(gSampler, uv).rgb * 2.0f - 1.0f;
        bumpSample.y = -bumpSample.y; // DX-convention

        ShadingFrame bumpFrame = CreateShadingFrame(Ns);
        Ns = bumpFrame.ToWorld(bumpSample);
    }
    Ns = q.CommittedTriangleFrontFace() == 0 ? -Ns : Ns;

    float4 albedoSample = gTextures[mat.TexIdxAlbedo].Sample(gSampler, uv);
    if (cGammaCorrection)
        albedoSample.xyz = pow(albedoSample.xyz, 2.2f);

    float3 emissionSample = gTextures[mat.TexIdxEmissive].Sample(gSampler, uv).rgb;

    if (cAnisotropyEnabled)
    {
        float2 anisoRot = float2(1, 0); // default value, unimplemented
        float2x2 anisoRotMat = float2x2(anisoRot.x,  anisoRot.y,
                                        -anisoRot.y,  anisoRot.x);

        anisoDirAndStrength = gTextures[mat.TexIdxAniso].Sample(gSampler, uv).rgb;
        anisoDirAndStrength.rg = anisoDirAndStrength.rg * 2.0f - 1.0f; // [0,1] -> [-1,1]. Don't normalize
        anisoDirAndStrength.rg = mul(anisoRotMat, anisoDirAndStrength.rg);

        float len2 = dot(anisoDirAndStrength.rg, anisoDirAndStrength.rg);
        anisoDirAndStrength.rg = (len2 > 1e-6f) ? anisoDirAndStrength.rg * rsqrt(len2) : float2(1, 0); // safe fallback
        anisoDirAndStrength.b *= mat.AnisoStrength;
    }

    if (cFurnaceTestHDR)
    {
        albedo = float4(0, 0, 0, 1);
        Li = float3(1, 1, 1);
        return;
    }

    if (cFurnaceTestHHE)
    {
        albedo = float4(1, 1, 1, 1);
        Li = float3(0, 0, 0);
        return;
    }

    albedo = float4(mat.BaseColorFactor * albedoSample.rgb, albedoSample.a);
    Li = mat.EmissiveStrength * mat.EmissiveColor * emissionSample;
}

#endif