#include "Path-Tracing/Hit.hlsli"
#include "Path-Tracing/Miss.hlsli"

#include "LightingModels.hlsli"

float3 Trace(inout RayQuery<RAY_FLAGS> q,
            uint flags,
            uint instanceMask,
            RayDesc ray,
            inout uint rngState)
{
    float3 Lo = float3(0, 0, 0);
    float3 throughput = float3(1, 1, 1);

#ifdef DEBUG_BUFFER
#include "Path-Tracing/DebugBuffersPreTrace.hlsli"
#endif

    for (uint i = 0; i <= c_pathTracing.NumBounces; i++)
    {
        q.TraceRayInline(gTLAS, flags, instanceMask, ray);
        q.Proceed();

        if (q.CommittedStatus() != COMMITTED_TRIANGLE_HIT)
        {
#ifdef DEBUG_BUFFER
#include "Path-Tracing/DebugBuffersOnMiss.hlsli"
#endif
            float3 Li = Miss(ray.Origin, ray.Direction);
            Lo += Li;
            break;
        }

        float3 brdf = 0, outNg = 0, outNs = 0, Li = 0;
        PtMaterialData mat;
        Hit(rngState, brdf, outNg, outNs, Li, mat, q);

        float3 hitPos = ray.Origin + ray.Direction * q.CommittedRayT();

        float3 wo = ray.Direction;

#if defined(LIGHTING_LAMB_DIFF)
        Model_LambertionDiffuse(rngState, Lo, throughput, outNs, Li, brdf, ray.Direction);
#elif defined(LIGHTING_GLOSSY)
        Model_Glossy(rngState, Lo, throughput, mat.DiffuseProbability, mat.Roughness, outNs, Li, brdf, wo, ray.Direction);
#elif defined(LIGHTING_GLASS)
        if (mat.Flags & PtMaterialFlags::eIsGlass)
        {
            bool entering = q.CommittedTriangleFrontFace()!=0;
            Model_Glass(rngState, Lo, throughput, mat, entering, q.CommittedRayT(), outNs, Li, brdf, wo, ray.Direction);
        }
        else
            Model_Glossy(rngState, Lo, throughput, mat.DiffuseProbability, mat.Roughness, outNs, Li, brdf, wo, ray.Direction);
#endif

        ray.Origin = hitPos + outNg * EPSILON * sign(dot(outNg, ray.Direction));

#ifdef DEBUG_BUFFER
#include "Path-Tracing/DebugBuffersOnHit.hlsli"
#endif
    }

#ifdef DEBUG_BUFFER
#include "Path-Tracing/DebugBuffersPostTrace.hlsli"
#endif

    return Lo;
}