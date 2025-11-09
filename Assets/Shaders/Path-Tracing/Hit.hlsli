#ifndef H_HITPS_H
#define H_HITPS_H

#include "DebugPalette.hlsli"
#include "Rand01.hlsli"

void Hit(inout uint rngState, out float3 materialColor, out float3 Ng, out float3 Ns, out float3 light, inout RayQuery<RAY_FLAGS> q)
{
    PtInstanceData instance = gInstances[q.CommittedInstanceIndex()];
    PtMaterialData material = gMaterials[instance.MaterialIdx];

    uint3 tri = gIndexMegaBuffer[instance.IndexBufferOffset + q.CommittedPrimitiveIndex()];
    Vertex v0 = gVertexMegaBuffer[instance.VertexBufferOffset + tri.x];
    Vertex v1 = gVertexMegaBuffer[instance.VertexBufferOffset + tri.y];
    Vertex v2 = gVertexMegaBuffer[instance.VertexBufferOffset + tri.z];

    float2 barycentrics = q.CommittedTriangleBarycentrics();
    precise float3 bary = float3(1 - barycentrics.x - barycentrics.y, barycentrics.x, barycentrics.y);

	float3 p0 = mul(instance.M, float4(v0.position,1)).xyz;
	float3 p1 = mul(instance.M, float4(v1.position,1)).xyz;
	float3 p2 = mul(instance.M, float4(v2.position,1)).xyz;
	Ng = normalize( cross(p1 - p0, p2 - p0) );
	if(q.CommittedTriangleFrontFace()==0) Ng = -Ng;

    Ns = v0.normal * bary.x + v1.normal * bary.y + v2.normal * bary.z;
    Ns = mul((float3x3)instance.MTI, Ns);
    Ns = q.CommittedTriangleFrontFace() == 0 ? -Ns : Ns;
    Ns = normalize(Ns);

#if defined(FURNACE_TEST_EMISSIVE)
    materialColor = float3(0, 0, 0);
    light = float3(1, 1, 1);
#elif defined(FURNACE_TEST_CLASSIC)
    materialColor = float3(1, 1, 1);
    light = float3(0, 0, 0);
#else
    materialColor = material.BaseColorFactor;
    light = material.EmissiveStrength;
#endif
}

#endif