if (c_debug.DebugIdx == DebugBuffer::eRNG)
    return Rand01(DIM_JITTER_X, globalSampleIdx, rngState).xxx;
else if (c_debug.DebugIdx == DebugBuffer::eRNG2D)
    return float3(Rand01(DIM_JITTER_X, globalSampleIdx, rngState), Rand01(DIM_JITTER_Y, globalSampleIdx, rngState), 0);

uint prevInstanceID = 245345843754375u;
uint prevPrimID = 119378274847u;