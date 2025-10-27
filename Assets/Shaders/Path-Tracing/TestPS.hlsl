#include "CBV.h"

struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

ConstantBuffer<CbvPathTracing> c_pathTracing : register(b0);
RaytracingAccelerationStructure TLAS : register(t0);

float4 PSMain(VsOut input) : SV_Target0
{
    // Instantiate ray query object.
    // Template parameter allows driver to generate a specialized
    // implementation.
    RayQuery<RAY_FLAG_CULL_NON_OPAQUE |
             RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES |
             RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;

    uint myRayFlags =   RAY_FLAG_CULL_NON_OPAQUE |
                        RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES |
                        RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH |
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
        TLAS,
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
        /*ShadeMyTriangleHit(
            q.CommittedInstanceIndex(),
            q.CommittedPrimitiveIndex(),
            q.CommittedGeometryIndex(),
            q.CommittedRayT(),
            q.CommittedTriangleBarycentrics(),
            q.CommittedTriangleFrontFace() );*/
        return float4(1, 1, 1, 1);
    }

    // Do miss shading
    /*MyMissColorCalculation(
        q.WorldRayOrigin(),
        q.WorldRayDirection());*/
    return float4(0, 0, 0, 1);
}