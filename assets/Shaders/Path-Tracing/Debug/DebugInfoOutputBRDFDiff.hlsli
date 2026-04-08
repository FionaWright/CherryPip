if (!hasDebugOutput)
{
    hasDebugOutput = true;
    if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetBrdfDiff)
        debug = diffuseBrdf;
    else if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetPdfDiff)
        debug = pdf.xxx;
    else if (cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetVecHalf ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetD ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetF ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetG ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetBrdfSpec ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetAlpha ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoDir ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoStrength ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoAlpha ||
            cbvDebug.DebugIdx == DebugInfoOutput::eMicrofacetPdfSpec)
        debug = 0;
    else
    {
        hasDebugOutput = false;
        debug = 0;
    }
}