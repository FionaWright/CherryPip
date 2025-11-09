#ifndef H_CBV_H
#define H_CBV_H

#include "HlslGlue.h"

struct CbvMatrices
{
    float4x4 M; // Model
    float4x4 MTI; // Model Transpose Inverse (For Normals)
    float4x4 V; // View
    float4x4 P; // Projection
};

struct CbvPathTracing
{
    float4x4 InvP;
    float4x4 InvV;
    float3 CameraPositionWorld;
    uint NumBounces;

    uint RESERVED;
    uint SPP;
    uint NumFrames;
    uint AccumulationEnabled;

    uint WindowAppGuiWidth;
    uint UpdateAccumulation;
    float2 Jitter;
};

enum DebugBuffer
{
    eNormals,
    eBaseColor,
    eHitPos,
    eFirstBounceDirection,
    eMissHit,
    eHitDistRay0,
    eHitDistRay1,
    eMaterialID,
    eRNG,
    eSelfIntersection,
    eNaN,

    // Unimplemented for now:
    eDirectLighting,
    eIndirectLighting,
    eMetalness,
    eRoughness
};

struct CbvPathTracingDebug
{
    DebugBuffer DebugIdx;
    uint3 p;
};

struct CbvHighlightPixel
{
    uint2 SelectedPixelCoords;
    uint2 p;
};

#endif