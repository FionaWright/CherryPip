// https://sakibsaikia.github.io/graphics/2022/01/04/Nan-Checks-In-HLSL.html
bool IsNaN(float x)
{
    return (asuint(x) & 0x7fffffff) > 0x7f800000;
}