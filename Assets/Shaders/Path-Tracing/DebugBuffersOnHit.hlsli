if (c_debug.DebugIdx == DebugBuffer::eNormals)
    return outNormal;
else if (c_debug.DebugIdx == DebugBuffer::eFirstBounceDirection)
    return newDir;
else if (c_debug.DebugIdx == DebugBuffer::eHitPos)
    return hitPos;
else if (c_debug.DebugIdx == DebugBuffer::eBaseColor)
    return outMaterialColor;
else if (c_debug.DebugIdx == DebugBuffer::eMissHit)
    return float3(1, 1, 1);
else if (c_debug.DebugIdx == DebugBuffer::eHitDistRay0)
    return float(q.CommittedRayT()).xxx / 10.0;
else if (i == 1 && c_debug.DebugIdx == DebugBuffer::eHitDistRay1)
    return float(q.CommittedRayT()).xxx;
else if (c_debug.DebugIdx == DebugBuffer::eMaterialID)
    return Palette(gInstances[q.CommittedInstanceIndex()].MaterialIdx).rgb;