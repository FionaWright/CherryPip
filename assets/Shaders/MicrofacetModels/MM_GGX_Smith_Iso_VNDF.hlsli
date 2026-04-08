#ifndef H_GGX_SMITH_ISO_H
#define H_GGX_SMITH_ISO_H

#define MICROFACET_MODEL_CHOSEN

// The Big One, old:
// https://inria.hal.science/hal-00996995v2

// Simple Modern:
// https://jcgt.org/published/0007/04/01/paper.pdf

// Simple Smith Iso:
// https://auzaiffe.wordpress.com/2024/04/15/vndf-importance-sampling-an-isotropic-distribution/

// Also smith, recent:
// https://arxiv.org/pdf/2306.05044

// Bounded, more advanced?
// https://dl.acm.org/doi/abs/10.1145/3651291

struct MicrofacetModel
{
#include "MicrofacetModels/IMicrofacetModel.hlsli"

    void Init(float roughness, RngInfo rngInfo, float3 V);
    float m_alpha;
    float m_u3;
    float3 m_V;

    float3 m_H;
};

void MicrofacetModel::Init(float roughness, RngInfo rngInfo, float3 V)
{
    m_alpha = RoughnessToAlpha(roughness);
    //m_u3 = Rand01_Bounce(DIM_D_BSDF_U3, rngInfo);
    m_V = V;
}

float MicrofacetModel::RoughnessToAlpha(float roughness)
{
    return roughness * roughness;
}

// V and N are in world space
float3 SampleP22(float u1, float u2, float3 V, float alpha, float3 N)
{
    //if (alpha == 0.0f) // ?
    //    return N;

    // decompose the floattor in parallel and perpendicular components
    float3 wi_z = -N * dot(V, N);
    float3 wi_xy = V + wi_z;

    // warp to the hemisphere configuration
    float3 wiStd = -normalize(alpha * wi_xy + wi_z);

    // sample a spherical cap in (-wiStd.z, 1]
    float wiStd_z = dot(wiStd, N);
    float z = 1.0 - u2 * (1.0 + wiStd_z);
    float sinTheta = sqrt(saturate(1.0f - z * z));
    float phi = 2.0f * PI * u1 - PI;
    float x = sinTheta * cos(phi);
    float y = sinTheta * sin(phi);
    float3 cStd = float3(x, y, z);

    // reflect sample to align with normal
    float3 up = float3(0, 0, 1.000001); // Used for the singularity
    float3 wr = N + up;

    //float3 c = dot(wr, cStd) * wr / wr.z - cStd;
    float wrz_safe = max(wr.z, 1e-6f);
    float3 c = dot(wr, cStd) * wr / wrz_safe - cStd;

    // compute halfway direction as standard normal
    float3 wmStd = c + wiStd;
    float3 wmStd_z = N * dot(N, wmStd);
    float3 wmStd_xy = wmStd_z - wmStd;

    // return final normal
    return normalize(alpha * wmStd_xy + wmStd_z);
}

// https://inria.hal.science/hal-00996995v2
float3 MicrofacetModel::Sample(float u1, float u2)
{
    // 1. Aniso Stretch (Ignored)

    // 2. Sample P22
    float3 N = float3(0, 0, 1);
    float3 H = SampleP22(u1, u2, m_V, m_alpha, N); // V and N are in shading space, problem?

    // 3. Aniso Unstretch (Ignored)

    return H;
}

float MicrofacetModel::D(float3 H)
{
    float a2 = m_alpha * m_alpha;
    float NdH = H.z;

    float denominator = (NdH * NdH * (a2 - 1.0f) + 1.0f);
    return a2 / max(0.001f, PI * denominator * denominator);
}

float MicrofacetModel::G1(float3 W)
{
    float a2 = m_alpha * m_alpha;
    float NdW = W.z;

    float denom = NdW + sqrt(max(0.0f, a2 + (1-a2) * NdW * NdW));
    return saturate(2 * NdW / max(0.001f, denom));
}

float MicrofacetModel::G2(float3 L, float3 V)
{
    return G1(L) * G1(V);
}

float MicrofacetModel::PDF(float D, float3 H, float3 V)
{
    float a2 = m_alpha * m_alpha;
    float NdV = V.z;
    float NdH = H.z;
    float VdH = dot(V, H);

    float Dv = D * G1(V) * VdH / max(1e-6f, NdV);
    return Dv / (4.0f * max(0.001f, VdH));

    //float nrm = rsqrt((NdV * NdV) * (1.0f - a2) + a2);
    //float sigmaStd = (NdV * nrm) * 0.5f + 0.5f;
    //float sigmaI = sigmaStd / max(nrm, 1e-6f);
    //float nrmN = (NdH * NdH) * (a2 - 1.0f) + 1.0f;
    //return a2 / max(1e-6f, PI * 4.0f * nrmN * nrmN * sigmaI);
}

#endif