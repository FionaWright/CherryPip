hasDebugOutput = true;
if (c_debug.DebugIdx == DebugBuffer::eMicrofacetD)
    debug = D.xxx;
else if (c_debug.DebugIdx == DebugBuffer::eMicrofacetF)
    debug = F;
else if (c_debug.DebugIdx == DebugBuffer::eMicrofacetG)
    debug = G.xxx;
else if (c_debug.DebugIdx == DebugBuffer::eMicrofacetAlpha)
    debug = alpha.xxx;
else if (c_debug.DebugIdx == DebugBuffer::eMicrofacetBrdfSpec)
    debug = specularBrdf;
else if (c_debug.DebugIdx == DebugBuffer::eMicrofacetSpecProb)
    debug = specProb.xxx;
else if (c_debug.DebugIdx == DebugBuffer::eMicrofacetPdfSpec)
    debug = pdf.xxx;
else if (c_debug.DebugIdx == DebugBuffer::eMicrofacetVecHalf)
    debug = H_s;
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
else if (c_debug.DebugIdx == DebugBuffer::eMicrofacetBrdfDiff ||
        c_debug.DebugIdx == DebugBuffer::eMicrofacetPdfDiff)
    debug = 0;
else
{
    hasDebugOutput = false;
    debug = 0;
}