if (c_debug.DebugIdx == DebugBuffer::eMissHit)
    return float3(0, 0, 0);
else if (c_debug.DebugIdx == DebugBuffer::eHitDistRay0)
    return float3(1, 1, 1);
else if (i == 1 && c_debug.DebugIdx == DebugBuffer::eHitDistRay1)
    return float3(1, 1, 1);