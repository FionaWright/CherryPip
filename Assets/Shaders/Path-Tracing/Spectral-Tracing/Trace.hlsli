#include "MathUtils.hlsli"

float3 Trace(inout RayQuery<RAY_FLAGS> q,
            uint flags,
            uint instanceMask,
            RayDesc ray,
            inout RngInfo rngInfo)
{
    Spectrum Lo = BlackSpectrum();
    Spectrum throughput = WhiteSpectrum();

    for (uint i = 0; i <= cbvPathTracing.NumBounces; i++)
    {
        q.TraceRayInline(gTLAS, flags, instanceMask, ray);
        q.Proceed();

        if (q.CommittedStatus() != COMMITTED_TRIANGLE_HIT)
        {
            Spectrum L_sample = throughput * Miss(ray.Origin, ray.Direction, i);
            Lo += L_sample;
            break;
        }

		float3 hitPos = ray.Origin + ray.Direction * q.CommittedRayT();

        StMaterialData mat;
		Spectrum albedo;
        Spectrum Li;
        float3 Ng = -1, Ns = -1;
        float2 uv = -1;
        Hit(q, albedo, Ng, Ns, Li, mat, uv);

        float3 wo = -ray.Direction;
        float3 wi, L_sample;

        //Model_LambertionDiffuse(rngInfo, throughput, Ns, Li, albedo.rgb, wi, L_sample);
        {
            L_sample = throughput * Li;

            float3 T, B;
            BuildBasisFrisvad(Ns, T, B);

            float u1 = Rand01_Bounce(DIM_D_BSDF_U1, rngInfo);
            float u2 = Rand01_Bounce(DIM_D_BSDF_U2, rngInfo);

            wi = RandHemisphereCosineWorld(u1, u2, T, B, Ns);
            // diffuseBrdf = albedo / PI
            // pdf = NdL / PI
            // throughput *= diffuseBrdf * NdL / pdf
            throughput *= albedo; // Terms cancel out
        }

        Lo += L_sample;

        ray.Direction = wi;
        ray.Origin = hitPos + Ng * EPSILON * sign(dot(Ng, ray.Direction));
    }

    return Lo;
}