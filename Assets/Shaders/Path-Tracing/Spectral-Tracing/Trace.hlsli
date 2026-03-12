#ifndef H_TRACE_H
#define H_TRACE_H

#include "Path-Tracing/Spectral-Tracing/Hit.hlsli"
#include "Path-Tracing/Spectral-Tracing/Miss.hlsli"
#include "Path-Tracing/Spectral-Tracing/SpectrumToRGB2019.hlsli"
#include "MathUtils.hlsli"

#include "LightingModels/AllSpectralModels.hlsli"

float3 Trace(inout RayQuery<RAY_FLAGS> q,
            uint flags,
            uint instanceMask,
            RayDesc ray,
            float lambda,
            inout RngInfo rngInfo)
{
    SpectralValue Lo = BlackSpectralValue();
    SpectralValue throughput = WhiteSpectralValue();

    for (uint i = 0; i <= cbvPathTracing.NumBounces; i++)
    {
        q.TraceRayInline(gTLAS, flags, instanceMask, ray);
        q.Proceed();

        if (q.CommittedStatus() != COMMITTED_TRIANGLE_HIT)
        {
            SpectralValue L_sample = Mul(throughput, Miss(ray.Origin, ray.Direction, i, lambda));
#ifdef SINGLE_LAMBDA_RENDERING
            Lo += L_sample;
#else
            Lo.Add(L_sample);
#endif
            break;
        }

		float3 hitPos = ray.Origin + ray.Direction * q.CommittedRayT();

        PtMaterialData mat;
		SpectralValue albedo;
        SpectralValue Li;
        float3 Ng = -1, Ns = -1;
        float2 uv = -1;
        Hit(q, lambda, albedo, Ng, Ns, Li, mat, uv);

        float2 roughMet = gTextures[mat.TexIdxRoughMet].Sample(gSampler, uv).gb;
        float roughness = mat.Roughness * roughMet.r;
        float metalness = mat.Metalness * roughMet.g;

        float3 wo = -ray.Direction;
        float3 wi;

        SpectralValue L_sample;

        if (cLightingGlassEnabled && mat.Flags & PtMaterialFlags::eIsGlass)
        {
            bool entering = q.CommittedTriangleFrontFace()!=0;
            Model_Glass_Spectral(rngInfo, throughput, mat.DiffuseProbability,
                    roughness, entering, q.CommittedRayT(), mat.IoR,
                    mat.GlassSigmaA, Ns, Li, albedo, lambda, wo, wi, L_sample);
        }
        else
            Model_LambertionDiffuse_Spectral(rngInfo, throughput, Ns, Li, albedo, wi, L_sample);

#ifdef SINGLE_LAMBDA_RENDERING
        Lo += L_sample;
#else
        Lo.Add(L_sample);
#endif

        ray.Direction = wi;
        ray.Origin = hitPos + Ng * EPSILON * sign(dot(Ng, ray.Direction));
    }

#ifdef SINGLE_LAMBDA_RENDERING
    float pdf = 1.0f / (float)VISIBLE_LIGHT_SPECTRUM_SIZE; // Assuming uniform sampling
    return SpectrumSampleToRGB(Lo, lambda, pdf);
#else
    return SpectrumToRGB(Lo);
#endif
}

#endif