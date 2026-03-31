#ifndef H_BRDF_SPECULAR_H
#define H_BRDF_SPECULAR_H

void BRDF_Specular(
    inout RngInfo rngInfo,
    inout float3 throughput,
    out float3 L_s,

    float roughness,
    float metalness,
    float3 albedo,
    float3 V_s,
    float3 N_s,
    float3 anisoDirAndStrength,
    ShadingFrame sframe,
    float3 F0,

    inout float3 debug,
    inout bool hasDebugOutput)
{
    float u1 = Rand01_Bounce(DIM_D_BSDF_U1, rngInfo);
    float u2 = Rand01_Bounce(DIM_D_BSDF_U2, rngInfo);

    MicrofacetModel mm;
    InitializeMM(mm, roughness, rngInfo, V_s);
    if (cAnisotropyEnabled)
        InitializeMMAniso(mm, T, B, N, anisoDirAndStrength);

    float3 H_s = normalize(mm.Sample(u1, u2));
    L_s = NormalizeSafe(reflect(-V_s, H_s), N_s);

    // Terminate ray if wi ends up inside surface
    if (L_s.z <= 0.0f)
    {
        throughput *= 0.0f;
        return;
    }

    float NdV = SSpaceCosTheta(V_s);
    float NdL = SSpaceCosTheta(L_s);
    float NdH = SSpaceCosTheta(H_s);
    float VdH = dot(H_s, V_s);

    float3 F = F_Schlick(VdH, F0);

    float D = mm.D(H_s);
    float G = mm.G2(L_s, V_s);
    float pdf = mm.PDF(D, H_s, V_s);

    float3 specularBrdf = 0.0f;
    if (roughness < 0.001f)
    {
        L_s = reflect(-V_s, N_s);
        specularBrdf = F;
        throughput *= specularBrdf;
    }
    else
    {
        specularBrdf = (D * G * F) / max(0.001f, 4 * NdV * NdL);
        throughput *= specularBrdf * NdL / max(0.001f, pdf);
    }

#ifdef DEBUG_PT_INFO_OUTPUT
#     include "Debug/DebugInfoOutputBRDFSpec.hlsli"
#endif
}

#endif