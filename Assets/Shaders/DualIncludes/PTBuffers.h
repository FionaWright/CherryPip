#ifndef H_PTBUFFERS_H
#define H_PTBUFFERS_H

#include "HlslGlue.h"

struct PtInstanceData
{
    uint IndexBufferOffset;
    uint VertexBufferOffset;
    uint MaterialIdx;
    uint p;

    float4x4 MTI;
};

struct PtMaterialData
{
    float3 BaseColorFactor;
    float EmissiveFactor;

    uint TextureIdx;
    uint NormalMapIdx;
    uint2 p;
};

struct Vertex
{
    float3 position;
    float2 uv;
    float3 normal;
    float3 tangent;
    float3 bitangent;
};

#endif