if (c_debug.DebugIdx == DebugBuffer::eNaN)
{
    uint3 isNaN3 = IsNaN(Lo) | isinf(Lo);
    return isNaN3.x || isNaN3.y || isNaN3.z ? float3(1, 0, 1) : Lo;
}