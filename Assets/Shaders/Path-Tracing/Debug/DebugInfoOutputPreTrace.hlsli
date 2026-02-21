if (cbvDebug.DebugIdx == DebugInfoOutput::eRNG)
    return Rand01(DIM_JITTER_X, rngInfo).xxx;
else if (cbvDebug.DebugIdx == DebugInfoOutput::eRNG2D)
    return float3(Rand01(DIM_JITTER_X, rngInfo), Rand01(DIM_JITTER_Y, rngInfo), 0);
else if (cbvDebug.DebugIdx == DebugInfoOutput::eRayDir)
    return ray.Direction;

uint prevInstanceID = 245345843754375u;
uint prevPrimID = 119378274847u;