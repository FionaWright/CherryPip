if (c_debug.DebugIdx == DebugBuffer::eRNG)
    return Rand01(DIM_JITTER_X, globalSampleIdx, rngState).xxx;

uint prevInstanceID = 245345843754375u;
uint prevPrimID = 119378274847u;