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
    float EmissiveStrength;

    float3 EmissiveColor;
    uint TexIdxAlbedo;

    uint TexIdxNormal;
    uint TexIdxRoughMet;
    uint TexIdxEmissive;
    uint TexIdxAniso;

    float Roughness;
    float Metalness;
    float DiffuseProbability;
    float IoR;

    PtMaterialFlags Flags;
    float AnisoStrength;
    float2 p;
};

struct Vertex
{
    float3 position;
    float2 uv;
    float3 normal;
};

#define PATH_VISUALIZATION_MAX_BOUNCES 21

struct DebugPathVisualizationStruct
{
    float3 WorldSpacePositionAtBounce[PATH_VISUALIZATION_MAX_BOUNCES]; // Max 21 bounces
    int NumPositionsSet;
};

#endif