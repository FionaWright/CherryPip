#ifndef H_SHADING_FRAME_H
#define H_SHADING_FRAME_H

#include "MathUtils.hlsli"

struct ShadingFrame
{
    void Init(float3 n);

    float3 ToLocal(float3 W);
    float3 ToWorld(float3 W_s);

    float3 T;
    float3 B;
    float3 N;
};

ShadingFrame CreateShadingFrame(float3 n)
{
    ShadingFrame frame;
    frame.Init(n);
    return frame;
}

void ShadingFrame::Init(float3 n)
{
    N = n;
    BuildBasisFrisvad(N, T, B);
}

float3 ShadingFrame::ToLocal(float3 W)
{
    return normalize(float3(dot(W, T), dot(W, B), dot(W, N)));
}

float3 ShadingFrame::ToWorld(float3 W_s)
{
    return normalize(W_s.x * T + W_s.y * B + W_s.z * N);
}

#endif