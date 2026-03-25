if (!hasDebugOutput)
{
    hasDebugOutput = true;
    if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetVecViewSSpace)
        debug = V_s;
    else if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetVecViewWSpace)
        debug = wo;
    else if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetVecLightSSpace)
        debug = L_s;
    else if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetTangent)
        debug = T;
    else if (cbvDebug.DebugIdx == DebugInfoOutput::eMircofacetBinormal)
        debug = B;
    else if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetVecHalf ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetD ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetF ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetG ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetBrdfSpec ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetAlpha ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoDir ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoStrength ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoAlpha ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetSpecProb ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetPdfSpec)
        debug = 0;
    else
    {
        hasDebugOutput = false;
        debug = 0;
    }
}