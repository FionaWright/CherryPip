#ifndef H_HITPS_H
#define H_HITPS_H

#include "DebugPalette.hlsli"
#include "Random.h"
#include "MathUtils.hlsli"

void Hit(inout RayQuery<RAY_FLAGS> q,
        out float4 albedo,
        out float3 Ng,
        out float3 Ns,
        out float3 Li,
        out PtMaterialData mat,
        out float2 uv
#ifdef ANISOTROPY_ENABLED
        , out float3 anisoDirAndStrength
#endif
)
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

    float3 N = v0.normal * bary.x + v1.normal * bary.y + v2.normal * bary.z;
    N = normalize(mul((float3x3)instance.MTI, N));
#ifdef NORMAL_MAPS_ENABLED
    float3 T, B;
    BuildBasisFrisvad(N, T, B);

    float3 bumpSample = gTextures[mat.TexIdxNormal].Sample(c_sampler, uv).rgb * 2.0f - 1.0f;
    bumpSample.y = -bumpSample.y; // DX-convention
    Ns = normalize(bumpSample.x * T + bumpSample.y * B + bumpSample.z * N);
#else
    Ns = N;
#endif
    Ns = q.CommittedTriangleFrontFace() == 0 ? -Ns : Ns;

    float4 albedoSample = gTextures[mat.TexIdxAlbedo].Sample(c_sampler, uv);
    // Does albedo sample need gamma correction?
    float3 emissionSample = gTextures[mat.TexIdxEmissive].Sample(c_sampler, uv).rgb;

#ifdef ANISOTROPY_ENABLED
    float2 anisoRot = float2(1, 0); // default value, unimplemented
    float2x2 anisoRotMat = float2x2(anisoRot.x,  anisoRot.y,
                                    -anisoRot.y,  anisoRot.x);

    anisoDirAndStrength = gTextures[mat.TexIdxAniso].Sample(c_sampler, uv).rgb;
    anisoDirAndStrength.rg = anisoDirAndStrength.rg * 2.0f - 1.0f; // [0,1] -> [-1,1]. Don't normalize
    anisoDirAndStrength.rg = mul(anisoRotMat, anisoDirAndStrength.rg);

    float len2 = dot(anisoDirAndStrength.rg, anisoDirAndStrength.rg);
    anisoDirAndStrength.rg = (len2 > 1e-6f) ? anisoDirAndStrength.rg * rsqrt(len2) : float2(1, 0); // safe fallback

    anisoDirAndStrength.b *= mat.AnisoStrength;
#endif

#if defined(FURNACE_TEST_HEMI_DIR_REFLECT)
    albedo = float4(0, 0, 0, 1);
    Li = float3(1, 1, 1);
#elif defined(FURNACE_TEST_HEMI_HEMI_EMIT)
    albedo = float4(1, 1, 1, 1);
    Li = float3(0, 0, 0);
#else
    albedo = float4(mat.BaseColorFactor * albedoSample.rgb, albedoSample.a);
    Li = mat.EmissiveStrength * mat.EmissiveColor * emissionSample;
#endif
}

#endif