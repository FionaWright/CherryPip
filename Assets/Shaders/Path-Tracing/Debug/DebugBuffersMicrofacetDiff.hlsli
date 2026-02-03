hasDebugOutput = true;
if (c_debug.DebugIdx == DebugBuffer::eMicrofacetBrdfDiff)
    debug = diffuseBrdf;
else if (c_debug.DebugIdx == DebugBuffer::eMicrofacetPdfDiff)
    debug = pdf.xxx;
else if (c_debug.DebugIdx == DebugBuffer::eMicrofacetSpecProb)
    debug = specProb.xxx;
else if (c_debug.DebugIdx == DebugBuffer::eMicrofacetVecViewSSpace)
    debug = V_s;
else if (c_debug.DebugIdx == DebugBuffer::eMicrofacetVecViewWSpace)
    debug = wo;
else if (c_debug.DebugIdx == DebugBuffer::eMicrofacetVecLightSSpace)
    debug = L_s;
else if (c_debug.DebugIdx == DebugBuffer::eMicrofacetTangent)
    debug = T;
else if (c_debug.DebugIdx == DebugBuffer::eMircofacetBinormal)
    debug = B;
else if (c_debug.DebugIdx == DebugBuffer::eMicrofacetVecHalf ||
        c_debug.DebugIdx == DebugBuffer::eMicrofacetD ||
        c_debug.DebugIdx == DebugBuffer::eMicrofacetF ||
        c_debug.DebugIdx == DebugBuffer::eMicrofacetG ||
        c_debug.DebugIdx == DebugBuffer::eMicrofacetBrdfSpec ||
        c_debug.DebugIdx == DebugBuffer::eMicrofacetAlpha ||
        c_debug.DebugIdx == DebugBuffer::eMicrofacetAnisoDir ||
        c_debug.DebugIdx == DebugBuffer::eMicrofacetAnisoStrength ||
        c_debug.DebugIdx == DebugBuffer::eMicrofacetAnisoAlpha ||
        c_debug.DebugIdx == DebugBuffer::eMicrofacetPdfSpec)
    debug = 0;
else
{
    hasDebugOutput = false;
    debug = 0;
}