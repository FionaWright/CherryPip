#include "Path-Tracing/Hit.hlsli"
#include "Path-Tracing/Miss.hlsli"

#include "LightingModels.hlsli"
#include "MathUtils.hlsli"

float3 Trace(inout RayQuery<RAY_FLAGS> q,
            uint flags,
            uint instanceMask,
            RayDesc ray,
            inout RngInfo rngInfo,
            uint sampleIdx, bool isPathVisualSelectedPixel)
{
    float3 Lo = float3(0, 0, 0);
    float3 throughput = float3(1, 1, 1);

#ifdef DEBUG_PT_INFO_OUTPUT
#    include "Debug/DebugInfoOutputPreTrace.hlsli"
#endif

    if (cDebugPathVisualizationEnabled && isPathVisualSelectedPixel)
	{
		gDebugPathVisualization[sampleIdx].WorldSpacePositionAtBounce[0] = ray.Origin;
		gDebugPathVisualization[sampleIdx].NumPositionsSet = 1;
	}

    for (uint i = 0; i <= cbvPathTracing.NumBounces; i++)
    {
        q.TraceRayInline(gTLAS, flags, instanceMask, ray);
        q.Proceed();

        if (q.CommittedStatus() != COMMITTED_TRIANGLE_HIT)
        {
            float3 L_sample = throughput * Miss(ray.Origin, ray.Direction, i);

            //float L_lum = Luminance(L_sample);
            //if (L_lum > cbvPathTracing.FireflyThreshold)
            //    L_sample *= cbvPathTracing.FireflyThreshold / L_lum;

            Lo += L_sample;

            if (cDebugPathVisualizationEnabled && isPathVisualSelectedPixel)
		    {
		    	uint bounceIdx = gDebugPathVisualization[sampleIdx].NumPositionsSet;
		        gDebugPathVisualization[sampleIdx].WorldSpacePositionAtBounce[bounceIdx] = ray.Origin + ray.Direction * 1000.0f;
		    }

#ifdef DEBUG_PT_INFO_OUTPUT
#    include "Debug/DebugInfoOutputOnMiss.hlsli"
#endif

            break;
        }

        if (cRngSamplingStrategy != eIndependent)
            rngInfo.BounceBaseDimension = GetBaseDim(i);

		float3 hitPos = ray.Origin + ray.Direction * q.CommittedRayT();

        PtMaterialData mat;
		float4 albedo = -1;
        float3 Ng = -1, Ns = -1, Li = -1;
        float2 uv = -1;
        float3 anisoDirAndStrength = -1;
        Hit(q, albedo, Ng, Ns, Li, mat, uv, anisoDirAndStrength);

        if (cAlphaTestingEnabled)
        {
            float rAlpha = Rand01_Bounce(DIM_D_ALPHA, rngInfo);
		    bool cutout = albedo.a < 0.001f || rAlpha > albedo.a;
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
        float3 wi, L_sample;

        if (cLightingGlassEnabled && mat.Flags & PtMaterialFlags::eIsGlass)
        {
            bool entering = q.CommittedTriangleFrontFace()!=0;
            Model_Glass(rngInfo, throughput, mat.DiffuseProbability, roughness, entering, q.CommittedRayT(), mat.IoR, Ns, Li, albedo.rgb, wo, wi, L_sample);
        }
        else if (cLightingModel == eLambert)
        {
            Model_LambertionDiffuse(rngInfo, throughput, Ns, Li, albedo.rgb, wi, L_sample);
        }
        else if (cLightingModel == eGlossy)
        {
            Model_Glossy(rngInfo, throughput, mat.DiffuseProbability, roughness, Ns, Li, albedo.rgb, wo, wi, L_sample);
        }
        else if (cLightingModel == eMicrofacet)
        {
            float3 debug = 0;
            bool hasDebugOutput = false;

            Model_Microfacet(rngInfo, throughput, roughness,
            	metalness, Ns, Li, albedo.rgb, anisoDirAndStrength, wo, wi, L_sample,
                debug, hasDebugOutput);

            if (cDebugInfoOutputEnabled && hasDebugOutput)
                return debug;
        }

        if (!cDebugInfoOutputEnabled && throughput.x <= 0 && throughput.y <= 0 && throughput.z <= 0)
            break;

        // TODO: Reimplement firefly threshold
        //float L_lum = Luminance(L_sample);
        //if (L_lum > cbvPathTracing.FireflyThreshold)
        //    L_sample *= cbvPathTracing.FireflyThreshold / L_lum;

        Lo += L_sample;

        if (cRussianRouletteEnabled && i >= cbvPathTracing.RussianRouletteMinBounces)
        {
            float p = saturate(max(throughput.r, max(throughput.g, throughput.b)));
            p = max(p, 0.05f);
            float rRR = PcgRand01(rngInfo.IndependentRngState);
            if (rRR > p)
                break;
            throughput /= p;
        }

        ray.Direction = wi;
        ray.Origin = hitPos + Ng * EPSILON * sign(dot(Ng, ray.Direction));

#ifdef DEBUG_PT_INFO_OUTPUT
#    include "Debug/DebugInfoOutputOnHit.hlsli"
#endif

        if (cDebugPathVisualizationEnabled && isPathVisualSelectedPixel)
		{
			uint bounceIdx = gDebugPathVisualization[sampleIdx].NumPositionsSet;
			gDebugPathVisualization[sampleIdx].WorldSpacePositionAtBounce[bounceIdx] = ray.Origin;
			gDebugPathVisualization[sampleIdx].NumPositionsSet += 1;
		}
    }

#ifdef DEBUG_PT_INFO_OUTPUT
#    include "Debug/DebugInfoOutputPostTrace.hlsli"
#endif

    return Lo;
}