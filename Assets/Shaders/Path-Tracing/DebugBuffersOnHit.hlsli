if (c_debug.DebugIdx == DebugBuffer::eNormalsShaded)
    return outNs;
else if (c_debug.DebugIdx == DebugBuffer::eNormalsGeo)
    return outNg;
else if (c_debug.DebugIdx == DebugBuffer::eFirstBounceDirection)
    return ray.Direction;
else if (c_debug.DebugIdx == DebugBuffer::eHitPos)
    return hitPos;
else if (c_debug.DebugIdx == DebugBuffer::eBaseColor)
    return brdf;
else if (c_debug.DebugIdx == DebugBuffer::eMissHit)
    return float3(1, 1, 1);
else if (c_debug.DebugIdx == DebugBuffer::eHitDistRay0)
    return float(q.CommittedRayT()).xxx / 10.0;
else if (i == 1 && c_debug.DebugIdx == DebugBuffer::eHitDistRay1)
    return float(q.CommittedRayT()).xxx;
else if (c_debug.DebugIdx == DebugBuffer::eMaterialID)
    return Palette(gInstances[q.CommittedInstanceIndex()].MaterialIdx).rgb;
else if (c_debug.DebugIdx == DebugBuffer::eSelfIntersection)
{
    uint instanceID = q.CommittedInstanceIndex();
    uint primID = q.CommittedPrimitiveIndex();
    if (instanceID == prevInstanceID && primID == prevPrimID)
        return float3(0, 1, 1);
    prevPrimID = primID;
    prevInstanceID = instanceID;
}