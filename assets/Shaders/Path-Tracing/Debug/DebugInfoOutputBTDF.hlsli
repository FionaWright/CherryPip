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
    else if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetVecHalf)
        debug = H_s;
    else if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetVecLightSSpace)
        debug = L_s;
    else if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoDir ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoStrength ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoAlpha)
        debug = 0;
    else if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetBrdfDiff ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetPdfDiff)
        debug = 0;
    else
    {
        hasDebugOutput = false;
        debug = 0;
    }
}