if (c_debug.DebugIdx == DebugInfoOutput::eMissHit)
    return float3(0, 0, 0);
else if (c_debug.DebugIdx == DebugInfoOutput::eHitDistRay0)
    return float3(1, 1, 1);
else if (i == 1 && c_debug.DebugIdx == DebugInfoOutput::eHitDistRay1)
    return float3(1, 1, 1);
//else if (c_debug.DebugIdx == DebugInfoOutput::eFireflyThresholdHit && L_lum > c_pathTracing.FireflyThreshold)
//    return float3(1, 0, 0);