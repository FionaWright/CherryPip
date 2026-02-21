#ifndef H_GGX_SMITH_ISO_H
#define H_GGX_SMITH_ISO_H

#define MICROFACET_MODEL_CHOSEN

// The Big One, old:
// https://inria.hal.science/hal-00996995v2

// Also smith, recent:
// https://arxiv.org/pdf/2306.05044

// Bounded, more advanced?
// https://dl.acm.org/doi/abs/10.1145/3651291

// https://github.com/mmp/pbrt-v3/blob/master/src/core/microfacet.cpp#L284

struct MicrofacetModel
{
#include "MicrofacetModels/IMicrofacetModel.hlsli"

    void Init(float roughness, RngInfo rngInfo, float3 V);
    void InitAniso(float3 T, float3 B, float3 N, float3 anisoDirStrength);
    float2 Sample11(float cosTheta, float u1, float u2);
    float Lambda(float3 W);

    float3 m_V;

    float m_alpha;
    float m_alphaX, m_alphaY;

    bool m_isAniso;
    float2 m_anisoDir;
    float3 m_anisoT, m_anisoB, m_anisoN;
};

void MicrofacetModel::Init(float roughness, RngInfo rngInfo, float3 V)
{
    m_alpha = RoughnessToAlpha(roughness);
    m_V = V;
}

void MicrofacetModel::InitAniso(float3 T, float3 B, float3 N, float3 anisoDirStrength)
{
    float2 alphaXY = AlphaToAnisoAlpha(m_alpha, anisoDirStrength.z);

    m_alphaX = alphaXY.x;
    m_alphaY = alphaXY.y;

    m_alphaX = max(m_alphaX, 1e-3);
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
}

float2 MicrofacetModel::Sample11(float cosTheta, float u1, float u2)
{
    // special case (normal incidence)
    if (cosTheta > .9999f)
    {
        float r = sqrt(u1 / (1 - u1));
        float phi = 6.28318530718 * u2;
        return float2(r * cos(phi), r * sin(phi));
    }

    float sinTheta = sqrt(max(0.0f, 1.0f - cosTheta * cosTheta));
    float tanTheta = sinTheta / cosTheta;
    float a = 1 / tanTheta;
    float G1 = 2 / (1 + sqrt(1.f + 1.f / (a * a)));

    // sample slope_x
    float A = 2 * u1 / G1 - 1;
    float tmp = 1.f / (A * A - 1.f);
    if (tmp > 1e10)
        tmp = 1e10;
    float B = tanTheta;
    float D = sqrt(max(B * B * tmp * tmp - (A * A - B * B) * tmp, 0.0f));
    float slope_x_1 = B * tmp - D;
    float slope_x_2 = B * tmp + D;
    float slopeX = (A < 0 || slope_x_2 > 1.f / tanTheta) ? slope_x_1 : slope_x_2;

    // sample slope_y
    float S;
    if (u2 > 0.5f)
    {
        S = 1.f;
        u2 = 2.f * (u2 - .5f);
    }
    else
    {
        S = -1.f;
        u2 = 2.f * (.5f - u2);
    }
    float z =
        (u2 * (u2 * (u2 * 0.27385f - 0.73369f) + 0.46341f)) /
        (u2 * (u2 * (u2 * 0.093073f + 0.309420f) - 1.000000f) + 0.597999f);
    float slopeY = S * z * sqrt(1.f + slopeX * slopeX);
    return float2(slopeX, slopeY);
}

// https://inria.hal.science/hal-00996995v2
float3 MicrofacetModel::Sample(float u1, float u2)
{
    bool flip = m_V.z < 0;
    if (flip)
        m_V = -m_V;

    // 1. stretch wi
    float3 wiStretched = normalize(float3(m_alphaX * m_V.x, m_alphaY * m_V.y, m_V.z));

    // 2. simulate P22_{wi}(x_slope, y_slope, 1, 1)
    float cosT = SSpaceCosTheta(wiStretched);
    float2 slope = Sample11(cosT, u1, u2);

    // 3. rotate
    float tmp = SSpaceCosPhi(wiStretched) * slope.x - SSpaceSinPhi(wiStretched) * slope.y;
    slope.y = SSpaceSinPhi(wiStretched) * slope.x + SSpaceCosPhi(wiStretched) * slope.y;
    slope.x = tmp;

    // 4. unstretch
    slope.x = m_alphaX * slope.x;
    slope.y = m_alphaY * slope.y;

    // 5. compute normal
    float3 H = normalize(float3(-slope.x, -slope.y, 1.));
    if (flip)
        H = -H;
    return H;
}

float MicrofacetModel::D(float3 H)
{
    float a2 = m_alpha * m_alpha;
    float NdH = H.z;

    float tan2Theta = SSpaceTan2Theta(H);
    if (isinf(tan2Theta))
        return 0.;

    float cos4Theta = SSpaceCos2Theta(H) * SSpaceCos2Theta(H);
    float e = (SSpaceCos2Phi(H) / (m_alphaX * m_alphaX) + SSpaceSin2Phi(H) / (m_alphaY * m_alphaY)) * tan2Theta;
    return 1 / (PI * m_alphaX * m_alphaY * cos4Theta * (1 + e) * (1 + e));
}

float MicrofacetModel::Lambda(float3 W)
{
    float absTanTheta = abs(SSpaceTanTheta(W));
    if (isinf(absTanTheta))
        return 0.;

    // Compute _alpha_ for direction _w_
    float alpha = sqrt(SSpaceCos2Phi(W) * m_alphaX * m_alphaX + SSpaceSin2Phi(W) * m_alphaY * m_alphaY);
    float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
    return (-1 + sqrt(1.f + alpha2Tan2Theta)) / 2.0f;
}

float MicrofacetModel::G1(float3 W)
{
    return 1.0f / (1.0f + Lambda(W));
}

float MicrofacetModel::G2(float3 L, float3 V)
{
    return 1.0f / (1.0f + Lambda(V) + Lambda(L));
}

float MicrofacetModel::PDF(float D, float3 H, float3 V)
{
    float a2 = m_alpha * m_alpha;
    float NdV = V.z;
    float NdH = H.z;
    float VdH = dot(V, H);

    float Dv = D * G1(V) * abs(VdH) / max(1e-6f, abs(NdV));
    return Dv / (4.0f * max(0.001f, VdH));

    //float nrm = rsqrt((NdV * NdV) * (1.0f - a2) + a2);
    //float sigmaStd = (NdV * nrm) * 0.5f + 0.5f;
    //float sigmaI = sigmaStd / max(nrm, 1e-6f);
    //float nrmN = (NdH * NdH) * (a2 - 1.0f) + 1.0f;
    //return a2 / max(1e-6f, PI * 4.0f * nrmN * nrmN * sigmaI);
}

#endif