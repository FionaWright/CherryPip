if (c_debug.DebugIdx == DebugBuffer::eNaN)
{
    return IsNaN3(Lo) || isinf(Lo.x) || isinf(Lo.y) || isinf(Lo.z) ? float3(1, 0, 1) : Lo;
}