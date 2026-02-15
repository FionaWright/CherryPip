hasDebugOutput = true;
if (c_debug.DebugIdx == DebugInfoOutput::eMicrofacetBrdfDiff)
    debug = diffuseBrdf;
else if (c_debug.DebugIdx == DebugInfoOutput::eMicrofacetPdfDiff)
    debug = pdf.xxx;
else if (c_debug.DebugIdx == DebugInfoOutput::eMicrofacetSpecProb)
    debug = specProb.xxx;
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
else if (c_debug.DebugIdx == DebugInfoOutput::eMicrofacetVecHalf ||
        c_debug.DebugIdx == DebugInfoOutput::eMicrofacetD ||
        c_debug.DebugIdx == DebugInfoOutput::eMicrofacetF ||
        c_debug.DebugIdx == DebugInfoOutput::eMicrofacetG ||
        c_debug.DebugIdx == DebugInfoOutput::eMicrofacetBrdfSpec ||
        c_debug.DebugIdx == DebugInfoOutput::eMicrofacetAlpha ||
        c_debug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoDir ||
        c_debug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoStrength ||
        c_debug.DebugIdx == DebugInfoOutput::eMicrofacetAnisoAlpha ||
        c_debug.DebugIdx == DebugInfoOutput::eMicrofacetPdfSpec)
    debug = 0;
else
{
    hasDebugOutput = false;
    debug = 0;
}