#ifndef H_GGX_SMITH_ANISO_H
#define H_GGX_SMITH_ANISO_H

#define MICROFACET_MODEL_CHOSEN

// https://github.com/Gforcex/LightingModel/blob/master/Assets/LightingModel/BRDF.cginc
// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_anisotropy/README.md

struct MicrofacetModel
{
#include "MicrofacetModels/IMicrofacetModel.hlsli"

    void Init(float roughness);
    void InitAniso(float3 anisoDirStrength);

    float m_alpha;
    float m_alphaX, m_alphaY;

    bool m_isAniso;
    float2 m_anisoDir;
    float3 m_anisoT, m_anisoB, m_anisoN;
};

void MicrofacetModel::Init(float roughness)
{
    m_alpha = RoughnessToAlpha(roughness);
}

void MicrofacetModel::InitAniso(float3 anisoDirStrength)
{
    float2 alphaXY = AlphaToAnisoAlpha(m_alpha, anisoDirStrength.z);

    m_alphaX = alphaXY.x;
    m_alphaY = alphaXY.y;

    //m_alphaX = clamp(m_alphaX, 1e-3, 1.0f);
    m_alphaX = max(m_alphaX, 1e-3);
    //m_alphaY = clamp(m_alphaY, 1e-3, 1.0f);
    m_alphaY = max(m_alphaY, 1e-3);

    m_isAniso = abs(m_alphaX - m_alphaY) > 0.0001f;
    if (m_isAniso)
    {
        m_anisoDir = anisoDirStrength.xy;

        m_anisoT = float3(m_anisoDir.xy, 0);
        m_anisoB = float3(-m_anisoDir.y, m_anisoDir.x, 0);
        m_anisoN = float3(0, 0, 1);
    }
}

float MicrofacetModel::RoughnessToAlpha(float roughness)
{
    return roughness * roughness;
    //return 0.5f * roughness * roughness + 0.5f * roughness;
}

float3 MicrofacetModel::Sample(float u1, float u2)
{
    if (!m_isAniso)
        return SampleH_GGX(m_alpha * m_alpha, u1, u2);

    float phi = atan(m_alphaY / m_alphaX * tan(2.0 * PI * u1));
    float sinPhi = sin(phi);
    float cosPhi = cos(phi);

    float alpha2 =
        1.0 / (cosPhi*cosPhi / (m_alphaX*m_alphaX) +
               sinPhi*sinPhi / (m_alphaY*m_alphaY));

    float cosTheta = sqrt((1.0 - u2) / (1.0 + (alpha2 - 1.0) * u2));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);

    return normalize(
        sinTheta * cosPhi * m_anisoT +
        sinTheta * sinPhi * m_anisoB +
        cosTheta * m_anisoN);
}

float MicrofacetModel::D(float3 H)
{
    float NdH = H.z;

    if (m_isAniso)
    {
        float k = H.x * H.x / (m_alphaX * m_alphaX) + H.y * H.y / (m_alphaY * m_alphaY) + H.z * H.z;
        return 1.0f / (PI * m_alphaX * m_alphaY * k*k);
    }

    float a2 = m_alpha * m_alpha;

    float denominator = (NdH * NdH * (a2 - 1.0f) + 1.0f);
    return a2 / max(0.001f, PI * denominator * denominator);
}

float G1CustomAlpha(float3 W, float alpha)
{
    float a2 = alpha * alpha;
    float NdW = W.z;

    float denom = NdW + sqrt(max(0.0f, a2 + (1-a2) * NdW * NdW));
    return saturate(2 * NdW / max(0.001f, denom));
}

float Lambda(float3 W, float alphaX, float alphaY)
{
    float aX2 = alphaX * alphaX;
    float aY2 = alphaY * alphaY;
    float k = (aX2 * W.x * W.x + aY2 * W.y * W.y) / max(0.0001f, W.z * W.z);
    return 0.5f * (-1 + sqrt(1 + k));
}

float MicrofacetModel::G1(float3 W)
{
    //return 1.0f / max(0.001f, 1 + Lambda(W, alphaX, alphaY));
    return G1CustomAlpha(W, m_alpha);
}

float MicrofacetModel::G2(float3 L, float3 V)
{
    if (m_isAniso)
    {
        return 1.0f / max(0.001f, 1 + Lambda(V, m_alphaX, m_alphaY) + Lambda(L, m_alphaX, m_alphaY));
    }

    return G1(L) * G1(V);
}

float MicrofacetModel::PDF(float D, float3 H, float3 V)
{
    float NdH = H.z;
    float VdH = dot(V, H);

    return D * NdH / (4.0f * max(0.001f, VdH));
}

#endif