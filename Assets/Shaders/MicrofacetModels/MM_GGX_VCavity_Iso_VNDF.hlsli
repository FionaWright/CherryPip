#ifndef H_GGX_SMITH_ISO_H
#define H_GGX_SMITH_ISO_H

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
    m_u3 = Rand01_Bounce(DIM_D_BSDF_U3, rngInfo);
    m_V = V;
}

float MicrofacetModel::RoughnessToAlpha(float roughness)
{
    return roughness * roughness;
}

// https://inria.hal.science/hal-00996995v2
float3 MicrofacetModel::Sample(float u1, float u2)
{
    float a2 = m_alpha * m_alpha;

    float3 H = SampleH_GGX(a2, u1, u2);
    float3 Hp = float3(-H.x, -H.y, H.z);

    float3 L = normalize(reflect(-m_V, H));

    float cHdL = saturate(dot(H, L));
    float cHpdL = saturate(dot(Hp, L));

    if (m_u3 > cHdL / max(0.001f, cHdL + cHpdL))
        m_H = Hp;
    else
        m_H = H;
    return m_H;
}

float MicrofacetModel::D(float3 H)
{
    float a2 = m_alpha * m_alpha;
    float NdH = H.z;

    float denominator = (NdH * NdH * (a2 - 1.0f) + 1.0f);
    return a2 / max(0.001f, PI * denominator * denominator);
}

// De-generalized from BSDF to BRDF
float MicrofacetModel::G1(float3 W)
{
    float WdH = dot(W, m_H);
    return min(1, 2.0f * m_H.z * W.z / max(0.001f, WdH));
}

// Avoid Keleman simplified model, breaks VNDF
float MicrofacetModel::G2(float3 L, float3 V)
{
    return min(G1(L), G1(V));
}

// Nonsense, find actual PDF
float MicrofacetModel::PDF(float D, float3 H, float3 V)
{
    float a2 = m_alpha * m_alpha;
    float NdH = H.z;
    float NdV = V.z;
    float VdH = dot(V, H);

    float Dv = D * G1(V) * max(0.0f, VdH) / max(0.001f, NdV);
    return Dv / (4.0f * max(0.001f, VdH));
}

#endif