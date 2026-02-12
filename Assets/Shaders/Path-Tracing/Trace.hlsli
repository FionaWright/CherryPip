#include "Path-Tracing/Hit.hlsli"
#include "Path-Tracing/Miss.hlsli"

#include "LightingModels.hlsli"

float3 Trace(inout RayQuery<RAY_FLAGS> q,
            uint flags,
            uint instanceMask,
            RayDesc ray,
            uint rngSampleIdx,
            inout uint rngState)
{
    float3 Lo = float3(0, 0, 0);
    float3 throughput = float3(1, 1, 1);

#ifdef DEBUG_BUFFER
#    include "Debug/DebugBuffersPreTrace.hlsli"
#endif

    for (uint i = 0; i <= c_pathTracing.NumBounces; i++)
    {
        q.TraceRayInline(gTLAS, flags, instanceMask, ray);
        q.Proceed();

        if (q.CommittedStatus() != COMMITTED_TRIANGLE_HIT)
        {
            float3 L_sample = throughput * Miss(ray.Origin, ray.Direction, i);

            //float L_lum = Luminance(L_sample);
            //if (L_lum > c_pathTracing.FireflyThreshold)
            //    L_sample *= c_pathTracing.FireflyThreshold / L_lum;

            Lo += L_sample;

#ifdef DEBUG_BUFFER
#    include "Debug/DebugBuffersOnMiss.hlsli"
#endif
            break;
        }

        uint rngBaseDimension = GetBaseDim(i);

		float3 hitPos = ray.Origin + ray.Direction * q.CommittedRayT();

        PtMaterialData mat;
		float4 albedo = -1;
        float3 Ng = -1, Ns = -1, Li = -1;
        float2 uv = -1;
        float3 anisoDirAndStrength = -1;
        Hit(q, albedo, Ng,
            Ns, Li, mat, uv
#ifdef ANISOTROPY_ENABLED
            , anisoDirAndStrength
#endif
        );

#ifdef ALPHA_TESTING_ENABLED
        float rAlpha = Rand01(rngBaseDimension + DIM_D_ALPHA, rngSampleIdx, rngState);
		bool cutout = albedo.a < 0.001f || rAlpha > albedo.a;
		if (cutout)
		{
        	ray.Origin = hitPos + Ng * EPSILON * sign(dot(Ng, ray.Direction));
			continue;
		}
#endif

        float2 roughMet = gTextures[mat.TexIdxRoughMet].Sample(c_sampler, uv).gb;
        float roughness = mat.Roughness * roughMet.r;
        float metalness = mat.Metalness * roughMet.g;

        float3 wo = -ray.Direction;
        float3 wi, L_sample;

#if defined(LIGHTING_LAMB_DIFF)

        Model_LambertionDiffuse(rngState, throughput, rngBaseDimension, rngSampleIdx, Ns, Li, albedo.rgb, wi, L_sample);

#elif defined(LIGHTING_GLOSSY)

#    ifdef LIGHTING_GLASS_ENABLED
        if (mat.Flags & PtMaterialFlags::eIsGlass)
        {
            bool entering = q.CommittedTriangleFrontFace()!=0;
            Model_Glass(rngState, throughput, rngBaseDimension, rngSampleIdx, mat.DiffuseProbability, roughness, entering, q.CommittedRayT(), mat.IoR, Ns, Li, albedo.rgb, wo, wi, L_sample);
        }
        else
#    endif
            Model_Glossy(rngState, throughput, rngBaseDimension, rngSampleIdx, mat.DiffuseProbability, roughness, Ns, Li, albedo.rgb, wo, wi, L_sample);

#elif defined(LIGHTING_MICROFACET)

        //if (dot(Ns, wo) <= 0) break;
        float3 debug = 0; bool hasDebugOutput = false;

#    ifdef LIGHTING_GLASS_ENABLED
		if (mat.Flags & PtMaterialFlags::eIsGlass)
        {
            bool entering = q.CommittedTriangleFrontFace()!=0;
            Model_Glass(rngState, throughput, rngBaseDimension, rngSampleIdx, mat.DiffuseProbability, roughness, entering, q.CommittedRayT(), mat.IoR, Ns, Li, albedo.rgb, wo, wi, L_sample);
        }
		else
#    endif
			Model_Microfacet(rngState, throughput, rngBaseDimension, rngSampleIdx, roughness,
            	metalness, Ns, Li, albedo.rgb, wo, wi, L_sample
#    ifdef ANISOTROPY_ENABLED
                , anisoDirAndStrength
#    endif
#    ifdef DEBUG_BUFFER
            	, debug
            	, hasDebugOutput
#    endif
        	);
        //if (dot(Ns, wi) <= 0) break;

#    ifdef DEBUG_BUFFER
        if (hasDebugOutput)
            return debug;
#    endif

#endif

        // TODO: Reimplement firefly threshold
        //float L_lum = Luminance(L_sample);
        //if (L_lum > c_pathTracing.FireflyThreshold)
        //    L_sample *= c_pathTracing.FireflyThreshold / L_lum;

        Lo += L_sample;

#if defined(RUSSIAN_ROULETTE_ENABLED)
        if (i >= c_pathTracing.RussianRouletteMinBounces) // Standard is 2-3
        {
            float p = saturate(max(throughput.r, max(throughput.g, throughput.b)));
            p = max(p, 0.05f);
            float rRR = Rand01(rngBaseDimension + DIM_D_RUSSIAN, rngSampleIdx, rngState);
            if (rRR > p)
                break;
            throughput /= p;
        }
#endif

        ray.Direction = wi;
        ray.Origin = hitPos + Ng * EPSILON * sign(dot(Ng, ray.Direction));

#ifdef DEBUG_BUFFER
#    include "Debug/DebugBuffersOnHit.hlsli"
#endif
    }

#ifdef DEBUG_BUFFER
#    include "Debug/DebugBuffersPostTrace.hlsli"
#endif

    return Lo;
}