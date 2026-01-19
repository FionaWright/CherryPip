#ifndef H_PTBUFFERS_H
#define H_PTBUFFERS_H

#include "HlslGlue.h"

struct PtInstanceData
{
    uint IndexBufferOffset;
    uint VertexBufferOffset;
    uint MaterialIdx;
    uint p;

    float4x4 M;
    float4x4 MTI;
};

enum PtMaterialFlags : uint
{
    eNone = 0,
    eIsGlass = 1
};

struct PtMaterialData
{
    float3 BaseColorFactor;
    float p;

    float3 EmissiveColor;
    float EmissiveStrength;

    uint TexIdxAlbedo;
    uint TexIdxNormal;
    uint TexIdxRoughMet;
    uint TexIdxEmissive;

    float Roughness;
    float Metalness;
    float DiffuseProbability;
    float IoR;

    PtMaterialFlags Flags;
    float3 p2;
};

struct Vertex
{
    float3 position;
    float2 uv;
    float3 normal;
    float3 tangent;
    float3 binormal;
};

#endif