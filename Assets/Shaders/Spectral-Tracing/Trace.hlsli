#ifndef H_TRACE_H
#define H_TRACE_H

#include "Spectral-Tracing/Hit.hlsli"
#include "Spectral-Tracing/Miss.hlsli"
#include "Spectral-Tracing/Spectrum/SpectrumToRGB2019.hlsli"
#include "MathUtils.hlsli"

#include "LightingModels/AllSpectralModels.hlsli"

#define RAYLEIGH_SCATTERING

float3 Trace(inout RayQuery<RAY_FLAGS> q,
            uint flags,
            uint instanceMask,
            RayDesc ray,
            SpectralContext ctx,
            inout RngInfo rngInfo)
{
    SpectralValue Lo = CreateBlackSpectralValue();
    SpectralValue throughput = CreateWhiteSpectralValue();

    for (uint i = 0; i <= cbvPathTracing.NumBounces; i++)
    {
        q.TraceRayInline(gTLAS, flags, instanceMask, ray);
        q.Proceed();

        if (q.CommittedStatus() != COMMITTED_TRIANGLE_HIT)
        {
            SpectralValue L_sample = Mul(throughput, Miss(ray.Origin, ray.Direction, i, ctx));
            Lo.Add(L_sample);

#ifdef DEBUG_PT_INFO_OUTPUT
#    include "Spectral-Tracing/Debug/DebugInfoOutputOnMiss.hlsli"
#endif
            break;
        }

        float hitDist = q.CommittedRayT();
		float3 hitPos = ray.Origin + ray.Direction * hitDist;

        PtMaterialData mat;
		SpectralValue albedo;
        SpectralValue Li;
        float3 Ng = -1, Ns = -1;
        float2 uv = -1;
        float albedoAlpha = -1;
        Hit(q, ctx, albedo, Ng, Ns, Li, mat, albedoAlpha, uv);

        if (cAlphaTestingEnabled)
        {
            float rAlpha = Rand01_Bounce(DIM_D_ALPHA, rngInfo);
            bool cutout = albedoAlpha < 0.001f || rAlpha > albedoAlpha;
            if (cutout)
            {
                ray.Origin = hitPos + Ng * EPSILON * sign(dot(Ng, ray.Direction));
                continue;
            }
        }

        float2 roughMet = gTextures[mat.TexIdxRoughMet].Sample(gSampler, uv).gb;
        float roughness = mat.Roughness * roughMet.r;
        float metalness = mat.Metalness * roughMet.g;

        float3 wo = -ray.Direction;
        float3 wi;

        SpectralValue L_sample;

        bool entering = q.CommittedTriangleFrontFace()!=0;
        bool isGlass = cLightingGlassEnabled && mat.Flags & PtMaterialFlags::eIsGlass;

        if (cLightingModel == eLambert)
        {
            Model_LambertionDiffuse_Spectral(rngInfo, throughput, Ns, Li, albedo, wi, L_sample);
        }
        else if (cLightingModel == eMicrofacet)
        {
            float3 debug = 0;
            bool hasDebugOutput = false;

            float3 anisoDirAndStrength = 0;

            Model_BSDF_Spectral(rngInfo, throughput, ctx, roughness, metalness,
                entering, isGlass, mat.GlassSigmaA, hitDist,
                Ns, Li, albedo, anisoDirAndStrength,
                wo, wi, L_sample,
                debug, hasDebugOutput);

            if (cDebugInfoOutputEnabled && hasDebugOutput)
                return debug;
        }

        if (!cDebugInfoOutputEnabled && throughput.IsBlack())
            break;

        Lo.Add(L_sample);

        if (cRussianRouletteEnabled && i >= cbvPathTracing.RussianRouletteMinBounces)
        {
            float p = saturate(throughput.Max());
            p = max(p, 0.05f);
            float rRR = PcgRand01(rngInfo.IndependentRngState);
            if (rRR > p)
                break;
            throughput.Div(p);
        }

        ray.Direction = wi;
        ray.Origin = hitPos + Ng * EPSILON * sign(dot(Ng, ray.Direction));

#ifdef DEBUG_PT_INFO_OUTPUT
#    include "Spectral-Tracing/Debug/DebugInfoOutputOnHit.hlsli"
#endif
    }

    return Lo.ToRGB(ctx);
}

#endif