#include "CBV.h"

struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

struct InstanceData
{
    uint IndexBufferIdx;
    uint VertexBufferIdx;
    uint MaterialIdx;
    uint p;

    float4x4 M;
};

struct Vertex
{
    float3 position;
    float2 uv;
    float3 normal;
    float3 tangent;
    float3 bitangent;
}

ConstantBuffer<CbvPathTracing> c_pathTracing : register(b0);
RaytracingAccelerationStructure gTLAS : register(t0);
StructuredBuffer<InstanceData> gInstances : register(t1);
StructuredBuffer<Vertex> gVertexBuffers[] : register(t2);
StructuredBuffer<uint3>  gIndexBuffers[]  : register(t3);

#include "Path-Tracing/HitPS.hlsli"

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
    ray.TMin = 0;
    ray.TMax = 1000.0;

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

    // Examine and act on the result of the traversal.
    // Was a hit committed?
    if (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
    {
        return Shade(
            q.CommittedInstanceIndex(),
            q.CommittedPrimitiveIndex(),
            q.CommittedGeometryIndex(),
            q.CommittedRayT(),
            q.CommittedTriangleBarycentrics(),
            q.CommittedTriangleFrontFace() );
    }

    // Do miss shading
    /*MyMissColorCalculation(
        q.WorldRayOrigin(),
        q.WorldRayDirection());*/
    return float4(0, 0, 0, 1);
}