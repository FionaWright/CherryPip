if (c_debug.DebugIdx == DebugInfoOutput::eNormalsShaded)
    return Ns;
else if (c_debug.DebugIdx == DebugInfoOutput::eNormalsGeo)
    return Ng;
else if (c_debug.DebugIdx == DebugInfoOutput::eFirstBounceDirection)
    return ray.Direction;
else if (c_debug.DebugIdx == DebugInfoOutput::eHitPos)
    return hitPos;
else if (c_debug.DebugIdx == DebugInfoOutput::eBaseColor)
    return albedo.rgb;
else if (c_debug.DebugIdx == DebugInfoOutput::eMissHit)
    return float3(1, 1, 1);
else if (c_debug.DebugIdx == DebugInfoOutput::eHitDistRay0)
    return float(q.CommittedRayT()).xxx / 10.0;
else if (i == 1 && c_debug.DebugIdx == DebugInfoOutput::eHitDistRay1)
    return float(q.CommittedRayT()).xxx;
else if (c_debug.DebugIdx == DebugInfoOutput::eMaterialID)
    return Palette(gInstances[q.CommittedInstanceIndex()].MaterialIdx).rgb;
else if (c_debug.DebugIdx == DebugInfoOutput::eSelfIntersection)
{
    uint instanceID = q.CommittedInstanceIndex();
    uint primID = q.CommittedPrimitiveIndex();
    if (instanceID == prevInstanceID && primID == prevPrimID)
        return float3(0, 1, 1);
    prevPrimID = primID;
    prevInstanceID = instanceID;
}
else if (c_debug.DebugIdx == DebugInfoOutput::eRoughness)
    return float3(roughness.rrr);
else if (c_debug.DebugIdx == DebugInfoOutput::eMetalness)
    return float3(metalness.rrr);
else if (c_debug.DebugIdx == DebugInfoOutput::eAlbedoAlpha)
    return float3(albedo.aaa);
//else if (c_debug.DebugIdx == DebugInfoOutput::eFireflyThresholdHit && L_lum > c_pathTracing.FireflyThreshold)
//    return float3(1, 0, 0);
