#ifndef H_TRACE_H
#define H_TRACE_H

#include "Path-Tracing/Spectral-Tracing/Hit.hlsli"
#include "Path-Tracing/Spectral-Tracing/SpectrumToRGB2019.hlsli"
#include "MathUtils.hlsli"

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
            //Spectrum L_sample = throughput * Miss(ray.Origin, ray.Direction, i);
            //Lo += L_sample;
            break;
        }

		float3 hitPos = ray.Origin + ray.Direction * q.CommittedRayT();

        PtMaterialData mat;
		SpectralValue albedo;
        SpectralValue Li;
        float3 Ng = -1, Ns = -1;
        float2 uv = -1;
        Hit(q, lambda, albedo, Ng, Ns, Li, mat, uv);

        float3 wo = -ray.Direction;
        float3 wi;

        SpectralValue L_sample;

        //Model_LambertionDiffuse(rngInfo, throughput, Ns, Li, albedo.rgb, wi, L_sample);
        {
            L_sample = Mul(throughput, Li);

            float3 T, B;
            BuildBasisFrisvad(Ns, T, B);

            float u1 = Rand01_Bounce(DIM_D_BSDF_U1, rngInfo);
            float u2 = Rand01_Bounce(DIM_D_BSDF_U2, rngInfo);

            wi = RandHemisphereCosineWorld(u1, u2, T, B, Ns);
            // diffuseBrdf = albedo / PI
            // pdf = NdL / PI
            // throughput *= diffuseBrdf * NdL / pdf
#ifdef SINGLE_LAMBDA_RENDERING // TODO: Make SpectralValue a struct with the same pattern as Spectrum maybe? Use polymorphism technique
            throughput *= albedo;
#else
            throughput.Mul(albedo); // Terms cancel out
#endif
        }

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