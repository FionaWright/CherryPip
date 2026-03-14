#ifndef H_HITPS_H
#define H_HITPS_H

#include "MathUtils.hlsli"

void Hit(inout RayQuery<RAY_FLAGS> q, // TODO: Why inout?
        float lambda,
        out SpectralValue albedo,
        out float3 Ng,
        out float3 Ns,
        out SpectralValue Li,
        out PtMaterialData mat,
        out float2 uv)
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
        float3 T, B;
        BuildBasisFrisvad(Ns, T, B);

        float3 bumpSample = gTextures[mat.TexIdxNormal].Sample(gSampler, uv).rgb * 2.0f - 1.0f;
        bumpSample.y = -bumpSample.y; // DX-convention
        Ns = normalize(bumpSample.x * T + bumpSample.y * B + bumpSample.z * Ns);
    }
    Ns = q.CommittedTriangleFrontFace() == 0 ? -Ns : Ns;

    // TODO: Albedo Alpha
    float4 albedoSRGB = gTextures[mat.TexIdxAlbedo].Sample(gSampler, uv);
    float3 albedoRGB = pow(albedoSRGB.xyz, 2.2f); // TODO: WRONG
    albedoRGB *= mat.BaseColorFactor;
    albedo.FromRGB(albedoRGB, eReflectance, lambda);

    float3 emissionSample = gTextures[mat.TexIdxEmissive].Sample(gSampler, uv).rgb;
    float3 emissionRGB = mat.EmissiveStrength * mat.EmissiveColor * emissionSample;
    Li.FromRGB(emissionRGB, eIlluminant, lambda);
}

#endif