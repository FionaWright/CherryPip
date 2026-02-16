#ifndef H_GGX_SMITH_ISO_H
#define H_GGX_SMITH_ISO_H

struct MicrofacetModel
{
#include "MicrofacetModels/IMicrofacetModel.hlsli"

    void Init(float roughness);
    float m_alpha;
};

void MicrofacetModel::Init(float roughness)
{
    m_alpha = RoughnessToAlpha(roughness);
}

float MicrofacetModel::RoughnessToAlpha(float roughness)
{
    return roughness * roughness;
}

float3 MicrofacetModel::Sample(float u1, float u2)
{
    float a2 = m_alpha * m_alpha;

    float phi = 2.0 * PI * u1;
    float cosTheta = sqrt(max(0.0f, (1.0 - u2) / max(0.001f, 1.0 + (a2 - 1.0) * u2)));
    float sinTheta = sqrt(max(0.0f, 1.0 - cosTheta * cosTheta));

    float3 H_s = normalize(float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta));
    return H_s;
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
    float NdH = H.z;
    float VdH = dot(V, H);

    return D * NdH / (4.0f * max(0.001f, VdH));
}

#endif