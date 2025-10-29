#include "CBV.h"
#include "PtBuffers.h"
#include "Rand01.hlsli"

#define EPSILON 1e-4

struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

ConstantBuffer<CbvPathTracing> c_pathTracing : register(b0);
RaytracingAccelerationStructure gTLAS : register(t0);
StructuredBuffer<PtInstanceData> gInstances : register(t1);
StructuredBuffer<Vertex> gVertexMegaBuffer : register(t2);
StructuredBuffer<uint3>  gIndexMegaBuffer  : register(t3);
StructuredBuffer<PtMaterialData> gMaterials  : register(t4);
RWTexture2D<float4> gAccum : register(u0);

#include "Path-Tracing/Hit.hlsli"
#include "Path-Tracing/Miss.hlsli"

#define RAY_FLAGS RAY_FLAG_CULL_NON_OPAQUE|RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES

float3 Trace(RayQuery<RAY_FLAGS> q, uint flags, uint instanceMask, RayDesc ray, inout uint rngState)
{
    float3 color = float3(0, 0, 0);
    float3 throughput = float3(1, 1, 1);

    for (uint i = 0; i < c_pathTracing.NumBounces; i++)
    {
        q.TraceRayInline(gTLAS, flags, instanceMask, ray);

        q.Proceed();

        if (q.CommittedStatus() != COMMITTED_TRIANGLE_HIT)
        {
            color += Miss(ray.Origin, ray.Direction);
            return color;
        }

        float3 newDir;
        float3 hitColor = Shade(throughput, rngState, newDir,
                            q.CommittedInstanceIndex(),
                            q.CommittedPrimitiveIndex(),
                            q.CommittedGeometryIndex(),
                            q.CommittedRayT(),
                            q.CommittedTriangleBarycentrics(),
                            q.CommittedTriangleFrontFace() );

        color += hitColor;

        float3 hitPos = ray.Origin + ray.Direction * q.CommittedRayT();
        ray.Direction = newDir;
        ray.Origin = hitPos + ray.Direction * max(EPSILON, EPSILON * (float)q.CommittedRayT());
    }

    return color;
}

float4 PSMain(VsOut input) : SV_Target0
{
    // Overkill?
    uint rngState = c_pathTracing.Seed;
    rngState ^= asuint(input.position.x) * 0x9E3779B9u;
    rngState ^= asuint(input.position.y) * 0x85EBCA6Bu;
    rngState ^= (rngState >> 13u);
    rngState *= 0xC2B2AE35u;

    RayQuery<RAY_FLAGS> q;

    uint flags = RAY_FLAGS|RAY_FLAG_CULL_BACK_FACING_TRIANGLES;

    uint instanceMask = 0xFF; // ?

    float3 origin = c_pathTracing.CameraPositionWorld;

    float2 ndc = input.uv * 2.0f - 1.0f; // [-1,1] range
    ndc.y = -ndc.y;
    float4 clip = float4(ndc, 0, 1); // z=0 for near plane
    float4 view = mul(c_pathTracing.InvP, clip);
    view /= view.w;
    float4 world = mul(c_pathTracing.InvV, view);
    float3 rayDir = normalize(world.xyz - origin);

    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = rayDir;
    ray.TMin = 0.001;
    ray.TMax = 1000.0;

    float3 colorSum = float3(0,0,0);
    for (uint i = 0; i < c_pathTracing.SPP; i++)
    {
        colorSum += Trace(q, flags, instanceMask, ray, rngState);
    }
    float4 finalColor = float4(colorSum / (float)c_pathTracing.SPP, 1);

    if (!c_pathTracing.AccumulationEnabled)
        return finalColor;

    input.position.x -= c_pathTracing.WindowAppGuiWidth;
    float4 accumColor = gAccum.Load(input.position.xy);
    float4 mu = (c_pathTracing.NumFrames * accumColor + finalColor) / (c_pathTracing.NumFrames+1);
    gAccum[input.position.xy] = mu;
    return mu;
}