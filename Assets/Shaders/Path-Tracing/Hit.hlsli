#ifndef H_HITPS_H
#define H_HITPS_H

#include "DebugPalette.hlsli"
#include "Rand01.hlsli"

// Use blasIdx to lookup transform/material info
// Use primitiveIdx in the index and vertex buffer to get 3 vertices
// Use geometryIdx if the blas contains submeshes
// HitPosWorld = Origin + rayT * Direction. Use for next ray bounce
// Use barycentrics to interpolate attribute data, get UV/normal/etc
float3 Shade(inout float3 throughput, inout uint rngState, inout float3 newDir, uint blasIdx, uint primitiveIdx, uint geometryIdx, float rayT, float2 barycentrics, uint isFrontFace)
{
    PtInstanceData instance = gInstances[blasIdx];
    PtMaterialData material = gMaterials[instance.MaterialIdx];

    uint3 tri = gIndexMegaBuffer[instance.IndexBufferOffset + primitiveIdx];
    Vertex v0 = gVertexMegaBuffer[instance.VertexBufferOffset + tri.x];
    Vertex v1 = gVertexMegaBuffer[instance.VertexBufferOffset + tri.y];
    Vertex v2 = gVertexMegaBuffer[instance.VertexBufferOffset + tri.z];

    float3 bary = float3(1 - barycentrics.x - barycentrics.y, barycentrics.x, barycentrics.y);

    float3 pos = v0.position * bary.x + v1.position * bary.y + v2.position * bary.z;
    float3 normal = normalize(v0.normal * bary.x + v1.normal * bary.y + v2.normal * bary.z);
    float3 tangent = normalize(v0.tangent * bary.x + v1.tangent * bary.y + v2.tangent * bary.z);
    float3 bitangent = normalize(v0.bitangent * bary.x + v1.bitangent * bary.y + v2.bitangent * bary.z);

    normal = isFrontFace == 0 ? -normal : normal;

    newDir = RandHemisphereCosine(rngState, normal, tangent, bitangent);

    if (dot(normal, newDir) < 0)
        return float3(1, 0, 1);

    throughput *= material.BaseColorFactor;
    return throughput * material.EmissiveFactor;
}

#endif

// Notes:
/*

For lambertian I need a Rand01() function and the ability to bounce
Lets start with with having the rays bounce directly away first then I'll implement a lambertian hemisphere function
Make sure it can be seeded!!

*/