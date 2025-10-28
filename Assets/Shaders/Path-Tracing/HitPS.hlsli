#ifndef H_HITPS_H
#define H_HITPS_H

#include "DebugPalette.hlsli"

// Use blasIdx to lookup transform/material info
// Use primitiveIdx in the index and vertex buffer to get 3 vertices
// Use geometryIdx if the blas contains submeshes
// HitPosWorld = Origin + rayT * Direction. Use for next ray bounce
// Use barycentrics to interpolate attribute data, get UV/normal/etc
float4 Shade(uint blasIdx, uint primitiveIdx, uint geometryIdx, float rayT, float2 barycentrics, uint isFrontFace)
{
    PtInstanceData instance = gInstances[blasIdx];
    //return Palette(instance.VertexBufferIdx);

    uint3 tri = gIndexMegaBuffer[instance.IndexBufferOffset + primitiveIdx];
    Vertex v0 = gVertexMegaBuffer[instance.VertexBufferOffset + tri.x];
    Vertex v1 = gVertexMegaBuffer[instance.VertexBufferOffset + tri.y];
    Vertex v2 = gVertexMegaBuffer[instance.VertexBufferOffset + tri.z];

    float3 bary = float3(1 - barycentrics.x - barycentrics.y, barycentrics.x, barycentrics.y);

    float3 pos = v0.position * bary.x + v1.position * bary.y + v2.position * bary.z;
    float3 normal = normalize(v0.normal * bary.x + v1.normal * bary.y + v2.normal * bary.z);
    normal = isFrontFace == 0 ? -normal : normal;

    return float4(normal, 1);
}

#endif