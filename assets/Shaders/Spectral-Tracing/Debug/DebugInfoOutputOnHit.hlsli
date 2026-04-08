if (cbvDebug.DebugIdx == DebugInfoOutput::eNormalsShaded)
    return Ns;
else if (cbvDebug.DebugIdx == DebugInfoOutput::eNormalsGeo)
    return Ng;
else if (cbvDebug.DebugIdx == DebugInfoOutput::eFirstBounceDirection)
    return ray.Direction;
else if (cbvDebug.DebugIdx == DebugInfoOutput::eHitPos)
    return hitPos;
else if (cbvDebug.DebugIdx == DebugInfoOutput::eBaseColor)
    return albedo.ToRGB(lambda);
else if (cbvDebug.DebugIdx == DebugInfoOutput::eMissHit)
    return float3(1, 1, 1);
else if (cbvDebug.DebugIdx == DebugInfoOutput::eHitDistRay0)
    return float(q.CommittedRayT()).xxx / 10.0;
else if (i == 1 && cbvDebug.DebugIdx == DebugInfoOutput::eHitDistRay1)
    return float(q.CommittedRayT()).xxx;
else if (cbvDebug.DebugIdx == DebugInfoOutput::eMaterialID)
    return Palette(gInstances[q.CommittedInstanceIndex()].MaterialIdx).rgb;
//else if (cbvDebug.DebugIdx == DebugInfoOutput::eSelfIntersection)
//{
//    uint instanceID = q.CommittedInstanceIndex();
//    uint primID = q.CommittedPrimitiveIndex();
//    if (instanceID == prevInstanceID && primID == prevPrimID)
//        return float3(0, 1, 1);
//    prevPrimID = primID;
//    prevInstanceID = instanceID;
//}
else if (cbvDebug.DebugIdx == DebugInfoOutput::eRoughness)
    return float3(roughness.rrr);
else if (cbvDebug.DebugIdx == DebugInfoOutput::eMetalness)
    return float3(metalness.rrr);
else if (cbvDebug.DebugIdx == DebugInfoOutput::eAlbedoAlpha)
    return float3(1, 0, 1); // TODO
//else if (cbvDebug.DebugIdx == DebugInfoOutput::eFireflyThresholdHit && L_lum > c_pathTracing.FireflyThreshold)
//    return float3(1, 0, 0);
