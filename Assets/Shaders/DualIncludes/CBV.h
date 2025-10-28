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

    uint Seed;
    uint3 p;
};

#endif