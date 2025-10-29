#ifndef H_HITPS_H
#define H_HITPS_H

#include "DebugPalette.hlsli"
#include "Rand01.hlsli"

void Hit(inout uint rngState, out float3 throughputMult, out float3 newDir, out float3 light, RayQuery<RAY_FLAGS> q)
{
    PtInstanceData instance = gInstances[q.CommittedInstanceIndex()];
    PtMaterialData material = gMaterials[instance.MaterialIdx];

    uint3 tri = gIndexMegaBuffer[instance.IndexBufferOffset + q.CommittedPrimitiveIndex()];
    Vertex v0 = gVertexMegaBuffer[instance.VertexBufferOffset + tri.x];
    Vertex v1 = gVertexMegaBuffer[instance.VertexBufferOffset + tri.y];
    Vertex v2 = gVertexMegaBuffer[instance.VertexBufferOffset + tri.z];

    float2 barycentrics = q.CommittedTriangleBarycentrics();
    precise float3 bary = float3(1 - barycentrics.x - barycentrics.y, barycentrics.x, barycentrics.y);

    precise float3 pos = v0.position * bary.x + v1.position * bary.y + v2.position * bary.z;
    precise float3 normal = v0.normal * bary.x + v1.normal * bary.y + v2.normal * bary.z;

    normal = mul((float3x3)instance.MTI, normal);
    normal = q.CommittedTriangleFrontFace() == 0 ? -normal : normal;
    normal = normalize(normal);

    throughputMult = material.BaseColorFactor;
    newDir = RandHemisphereCosine(rngState, normal);
    light = material.EmissiveFactor;
}

#endif