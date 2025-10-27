struct VsOut
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

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

    float3 worldPosition = float3(0,0,0); // Temp

    RayDesc ray;
    ray.Origin = worldPosition;
    ray.Direction = float3(0, 0, 1);
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
    else // COMMITTED_NOTHING
        // From template specialization,
            // COMMITTED_PROCEDURAL_PRIMITIVE can't happen.
    {
        // Do miss shading
        /*MyMissColorCalculation(
            q.WorldRayOrigin(),
            q.WorldRayDirection());*/
        return float4(0, 0, 0, 1);
    }

    return float4(1, 0, 0, 1);
}