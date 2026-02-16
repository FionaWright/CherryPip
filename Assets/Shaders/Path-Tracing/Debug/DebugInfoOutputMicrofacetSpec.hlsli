hasDebugOutput = true;
if (c_debug.DebugIdx == DebugInfoOutput::eMicrofacetD)
    debug = D.xxx;
else if (c_debug.DebugIdx == DebugInfoOutput::eMicrofacetF)
    debug = F;
else if (c_debug.DebugIdx == DebugInfoOutput::eMicrofacetG)
    debug = G.xxx;
else if (c_debug.DebugIdx == DebugInfoOutput::eMicrofacetAlpha)
    debug = mm.RoughnessToAlpha(roughness).xxx;
else if (c_debug.DebugIdx == DebugInfoOutput::eMicrofacetBrdfSpec)
    debug = specularBrdf;
else if (c_debug.DebugIdx == DebugInfoOutput::eMicrofacetSpecProb)
    debug = specProb.xxx;
else if (c_debug.DebugIdx == DebugInfoOutput::eMicrofacetPdfSpec)
    debug = pdf.xxx;
else if (c_debug.DebugIdx == DebugInfoOutput::eMicrofacetVecHalf)
    debug = H_s;
else if (c_debug.DebugIdx == DebugInfoOutput::eMicrofacetVecViewSSpace)
    debug = V_s;
else if (c_debug.DebugIdx == DebugInfoOutput::eMicrofacetVecViewWSpace)
    debug = wo;
else if (c_debug.DebugIdx == DebugInfoOutput::eMicrofacetVecLightSSpace)
    debug = L_s;
else if (c_debug.DebugIdx == DebugInfoOutput::eMicrofacetTangent)
    debug = T;
else if (c_debug.DebugIdx == DebugInfoOutput::eMircofacetBinormal)
    debug = B;
#ifdef ANISOTROPY_ENABLED
else if (c_debug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoDir)
    debug = float3(anisoDirAndStrength.xy, 0);
else if (c_debug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoStrength)
    debug = anisoDirAndStrength.zzz;
else if (c_debug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoAlpha)
    debug = float3(mm.m_alphaX, mm.m_alphaY, 0);
#else
else if (c_debug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoDir ||
        c_debug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoStrength ||
        c_debug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoAlpha)
    debug = 0;
#endif
else if (c_debug.DebugIdx == DebugInfoOutput::eMicrofacetBrdfDiff ||
        c_debug.DebugIdx == DebugInfoOutput::eMicrofacetPdfDiff)
    debug = 0;
else
{
    hasDebugOutput = false;
    debug = 0;
}