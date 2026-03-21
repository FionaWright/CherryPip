if (!hasDebugOutput)
{
    hasDebugOutput = true;
    if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetD)
        debug = D.xxx;
    else if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetF)
        debug = F;
    else if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetG)
        debug = G.xxx;
    else if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetAlpha)
        debug = mm.RoughnessToAlpha(roughness).xxx;
    else if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetBrdfSpec)
        debug = specularBrdf;
    else if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetPdfSpec)
        debug = pdf.xxx;
    else if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetVecHalf)
        debug = H_s;
#ifdef ANISOTROPY_ENABLED
    else if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoDir)
        debug = float3(anisoDirAndStrength.xy, 0);
    else if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoStrength)
        debug = anisoDirAndStrength.zzz;
    else if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoAlpha)
        debug = float3(mm.m_alphaX, mm.m_alphaY, 0);
#else
    else if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoDir ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoStrength ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoAlpha)
        debug = 0;
#endif
    else if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetBrdfDiff ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetPdfDiff)
        debug = 0;
    else
    {
        hasDebugOutput = false;
        debug = 0;
    }
}