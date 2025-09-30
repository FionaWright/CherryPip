#include "MathUtils.h"

#include <algorithm>
#include <iostream>
#include <fstream>

float Clamp(float val, float min, float max)
{
    return std::min(std::max(val, min), max);
}

XMFLOAT3 Add(XMFLOAT3 a, XMFLOAT3 b)
{
    return XMFLOAT3(a.x + b.x, a.y + b.y, a.z + b.z);
}

XMFLOAT4 Add(XMFLOAT4 a, XMFLOAT4 b)
{
    return XMFLOAT4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

XMFLOAT3 Subtract(XMFLOAT3 a, XMFLOAT3 b)
{
	return XMFLOAT3(a.x - b.x, a.y - b.y, a.z - b.z);
}

XMFLOAT2 Subtract(XMFLOAT2 a, XMFLOAT2 b)
{
	return XMFLOAT2(a.x - b.x, a.y - b.y);
}

XMFLOAT2 Frac(const XMFLOAT2& a)
{
    float fracX = a.x - std::floor(a.x);
    float fracY = a.y - std::floor(a.y);
    return XMFLOAT2(fracX, fracY);
}

XMFLOAT2 NaiveFrac(XMFLOAT2 a)
{
    a.x -= a.x < 0 ? std::ceil(a.x) : std::floor(a.x);
    a.y -= a.y < 0 ? std::ceil(a.y) : std::floor(a.y);
    return a;
}

XMFLOAT2 Abs(const XMFLOAT2& a)
{
    return XMFLOAT2(std::abs(a.x), std::abs(a.y));
}

XMFLOAT3 Abs(const XMFLOAT3& a)
{
    return XMFLOAT3(std::abs(a.x), std::abs(a.y), std::abs(a.z));
}

XMFLOAT3 Normalize(const XMFLOAT3& v)
{
	float length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	return XMFLOAT3(v.x / length, v.y / length, v.z / length);
}

XMFLOAT3 Divide(const XMFLOAT3& a, float d)
{
    return XMFLOAT3(a.x / d, a.y / d, a.z / d);
}

XMFLOAT3 Mult(const XMFLOAT3& a, float d)
{
    return XMFLOAT3(a.x * d, a.y * d, a.z * d);
}

XMFLOAT4 Mult(const XMFLOAT4& a, float d)
{
    return XMFLOAT4(a.x * d, a.y * d, a.z * d, a.w * d);
}

XMFLOAT3 Mult(const XMFLOAT3& a, const XMFLOAT3& b)
{
    return XMFLOAT3(a.x * b.x, a.y * b.y, a.z * b.z);
}

XMFLOAT3 Normalize(const XMFLOAT3& v, float& length)
{
    length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return XMFLOAT3(v.x / length, v.y / length, v.z / length);
}

XMFLOAT3 Negate(const XMFLOAT3& v)
{
    return XMFLOAT3(-v.x, -v.y, -v.z);
}

XMFLOAT3 Saturate(const XMFLOAT3& v)
{
    return XMFLOAT3(std::clamp(v.x, 0.0f, 1.0f), std::clamp(v.y, 0.0f, 1.0f), std::clamp(v.z, 0.0f, 1.0f));
}

float Magnitude(const XMFLOAT3& v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

float Dot(const XMFLOAT3& a, const XMFLOAT3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float SqDist(const XMFLOAT3& a, const XMFLOAT3& b)
{
    XMFLOAT3 a_b = Subtract(a, b);
    return Dot(a_b, a_b);
}

XMFLOAT3 Cross(const XMFLOAT3& a, const XMFLOAT3& b)
{
    XMFLOAT3 result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

bool Equals(XMFLOAT3 a, XMFLOAT3 b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

bool Equals(XMFLOAT2 a, XMFLOAT2 b)
{
    return a.x == b.x && a.y == b.y;
}

std::string ToString(const XMFLOAT3& v)
{
    return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
}

bool NextCharactersMatch(std::ifstream& file, const std::string& expected, bool resetPos)
{
    std::streampos startPosition = file.tellg();

    for (char c : expected)
    {
        if (file.peek() != c)
        {
            if (resetPos)
                file.seekg(startPosition);
            return false;
        }

        file.get();
    }

    if (resetPos)
        file.seekg(startPosition);
    return true;
}

bool XOR(bool a, bool b)
{
    return !a != !b;
}

bool Approx(float a, float b)
{
    return abs(a - b) < 0.001f;
}

float Rand01()
{
    return std::rand() / static_cast<float>(RAND_MAX);
}

float Rand(float min, float max)
{
    return min + (max - min) * Rand01();
}

uint32_t PackColor(XMFLOAT4 color)
{
    uint32_t r = (uint32_t)(color.x * 255.0f) & 0xFF;
    uint32_t g = (uint32_t)(color.y * 255.0f) & 0xFF;
    uint32_t b = (uint32_t)(color.z * 255.0f) & 0xFF;
    uint32_t a = (uint32_t)(color.w * 255.0f) & 0xFF;

    return (a << 24) | (b << 16) | (g << 8) | r;
}

uint32_t PackColor(XMFLOAT3 color)
{
    uint32_t r = (uint32_t)(color.x * 255.0f) & 0xFF;
    uint32_t g = (uint32_t)(color.y * 255.0f) & 0xFF;
    uint32_t b = (uint32_t)(color.z * 255.0f) & 0xFF;

    return (b << 16) | (g << 8) | r;
}

XMFLOAT4 UnpackColor4(uint32_t color)
{
    return XMFLOAT4((color & 0xFF) / 255.0f, ((color >> 8) & 0xFF) / 255.0f, ((color >> 16) & 0xFF) / 255.0f, ((color >> 24) & 0xFF) / 255.0f);
}

XMFLOAT3 UnpackColor3(uint32_t color)
{
    return XMFLOAT3((color & 0xFF) / 255.0f, ((color >> 8) & 0xFF) / 255.0f, ((color >> 16) & 0xFF) / 255.0f);
}