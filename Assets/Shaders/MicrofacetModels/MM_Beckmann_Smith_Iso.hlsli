#ifndef H_GGX_SMITH_ISO_H
#define H_GGX_SMITH_ISO_H

// Have a look at this:
// https://blog.selfshadow.com/publications/s2012-shading-course/hoffman/s2012_pbs_physics_math_notebook.pdf
// See PBRT for Aniso version

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

// Heuristic to have beckmann visually match GGX for the same roughness value
float MicrofacetModel::RoughnessToAlpha(float roughness)
{
    roughness = max(1e-3f, roughness);
    float x = log(roughness);
    return 1.62142f + 0.819955f * x + 0.1734f * x * x +
           0.0171201f * x * x * x + 0.000640711f * x * x * x * x;
}

float3 MicrofacetModel::Sample(float u1, float u2)
{
    float a2 = m_alpha * m_alpha;

    float phi = 2.0 * PI * u2;
    float tan2Theta = -a2 * log(1.0 - u1);
    float cosTheta = 1.0 / sqrt(1.0 + tan2Theta);
    float sinTheta = sqrt(max(0.0, 1.0 - cosTheta * cosTheta));

    float3 H_s = normalize(float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta));
    return H_s;
}

float MicrofacetModel::D(float3 H)
{
    float a2 = m_alpha * m_alpha;

    float tan2T = SSpaceTan2Theta(H);
    if (IsNaN(1/tan2T)) return 0; // Is infinite

    float cos4T = SSpaceCos2Theta(H) * SSpaceCos2Theta(H);
    float k3 = PI * a2 * cos4T;
    return exp(-tan2T / a2) / k3;
}

float MicrofacetModel::G1(float3 W)
{
    float a2 = m_alpha * m_alpha;
    float NdW = W.z;

    return -1;
}

float MicrofacetModel::G2(float3 L, float3 V)
{
    return -1;
}

// WRONG PROBABLY
float MicrofacetModel::PDF(float D, float3 H, float3 V)
{
    float NdH = H.z;
    float VdH = dot(V, H);

    return D * NdH / (4.0f * max(0.001f, VdH));
}

#endif