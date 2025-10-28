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

#include "Path-Tracing/Hit.hlsli"
#include "Path-Tracing/Miss.hlsli"

float4 PSMain(VsOut input) : SV_Target0
{
    // Instantiate ray query object.
    // Template parameter allows driver to generate a specialized
    // implementation.
    RayQuery<RAY_FLAG_CULL_NON_OPAQUE |
             RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES> q;

    uint myRayFlags =   RAY_FLAG_CULL_NON_OPAQUE |
                        RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES |
                        RAY_FLAG_CULL_BACK_FACING_TRIANGLES;

    uint myInstanceMask = 0xFF; // ?

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

    float4 color = float4(0, 0, 0, 1);
    float3 throughput = float3(1, 1, 1);

    // Overkill?
    uint rngState = c_pathTracing.Seed;
    rngState ^= asuint(input.position.x) * 0x9E3779B9u;
    rngState ^= asuint(input.position.y) * 0x85EBCA6Bu;
    rngState ^= (rngState >> 13u);
    rngState *= 0xC2B2AE35u;

    uint TEMP = 99;
    uint TEMP2 = 534346325;

    for (uint i = 0; i < c_pathTracing.NumBounces; i++)
    {
        // Set up a trace.  No work is done yet.
        q.TraceRayInline(
            gTLAS,
            myRayFlags, // OR'd with flags above
            myInstanceMask,
            ray);

        // Proceed() below is where behind-the-scenes traversal happens,
        // including the heaviest of any driver inlined code.
        // In this simplest of scenarios, Proceed() only needs
        // to be called once rather than a loop:
        // Based on the template specialization above,
        // traversal completion is guaranteed.
        q.Proceed();

        if (q.CommittedStatus() != COMMITTED_TRIANGLE_HIT)
        {
            color.rgb += Miss(ray.Origin, ray.Direction);
            return color;
        }

        uint temp = q.CommittedInstanceIndex();
        uint temp2 = q.CommittedPrimitiveIndex();
        //if (temp == TEMP && temp2 == TEMP2)
        //    return float4(0, 1, 1, 1);
        TEMP = temp;
        TEMP2 = temp2;

        float3 newDir;
        float3 hitColor = Shade(throughput, rngState, newDir,
                            q.CommittedInstanceIndex(),
                            q.CommittedPrimitiveIndex(),
                            q.CommittedGeometryIndex(),
                            q.CommittedRayT(),
                            q.CommittedTriangleBarycentrics(),
                            q.CommittedTriangleFrontFace() );

        color.rgb += hitColor;

        float3 hitPos = ray.Origin + ray.Direction * q.CommittedRayT();
        ray.Direction = newDir;
        ray.Origin = hitPos + ray.Direction * max(EPSILON, EPSILON * (float)q.CommittedRayT());
    }

    return color;
}