hasDebugOutput = true;
if (c_debug.DebugIdx == DebugBuffer::eMicrofacetD)
    debug = D.xxx;
else if (c_debug.DebugIdx == DebugBuffer::eMicrofacetF)
    debug = F;
else if (c_debug.DebugIdx == DebugBuffer::eMicrofacetG)
    debug = G.xxx;
else if (c_debug.DebugIdx == DebugBuffer::eMicrofacetBrdfSpec)
    debug = specularBrdf;
else if (c_debug.DebugIdx == DebugBuffer::eMicrofacetSpecProb)
    debug = specProb.xxx;
else if (c_debug.DebugIdx == DebugBuffer::eMicrofacetPdfSpec)
    debug = pdf.xxx;
else if (c_debug.DebugIdx == DebugBuffer::eMicrofacetHalfVec)
    debug = H_s;
else
    hasDebugOutput = false;