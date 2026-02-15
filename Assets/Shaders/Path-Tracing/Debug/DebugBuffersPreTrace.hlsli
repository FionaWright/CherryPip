if (c_debug.DebugIdx == DebugBuffer::eRNG)
    return Rand01(DIM_JITTER_X, rngInfo).xxx;
else if (c_debug.DebugIdx == DebugBuffer::eRNG2D)
    return float3(Rand01(DIM_JITTER_X, rngInfo), Rand01(DIM_JITTER_Y, rngInfo), 0);

uint prevInstanceID = 245345843754375u;
uint prevPrimID = 119378274847u;