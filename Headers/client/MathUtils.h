#pragma once
#include <string>

#include <DirectXMath.h>

using namespace DirectX;

#define PI 3.1415926535

struct Transform;

float Clamp(float val, float min, float max);

XMFLOAT3 Add(XMFLOAT3 a, XMFLOAT3 b);

XMFLOAT4 Add(XMFLOAT4 a, XMFLOAT4 b);

XMFLOAT3 Subtract(XMFLOAT3 a, XMFLOAT3 b);

XMFLOAT2 Subtract(XMFLOAT2 a, XMFLOAT2 b);

XMFLOAT2 Frac(const XMFLOAT2& a);

XMFLOAT2 NaiveFrac(XMFLOAT2 a);

XMFLOAT2 Abs(const XMFLOAT2& a);

XMFLOAT3 Abs(const XMFLOAT3& a);

XMFLOAT3 Normalize(const XMFLOAT3& v);

XMFLOAT3 Divide(const XMFLOAT3& a, float d);

XMFLOAT3 Mult(const XMFLOAT3& a, float d);

XMFLOAT4 Mult(const XMFLOAT4& a, float d);

XMFLOAT3 Mult(const XMFLOAT3& a, const XMFLOAT3& b);

float SqDist(const XMFLOAT3& a, const XMFLOAT3& b);

XMFLOAT3 Normalize(const XMFLOAT3& v, float& length);

XMFLOAT3 Negate(const XMFLOAT3& v);

XMFLOAT3 Saturate(const XMFLOAT3& v);

float Magnitude(const XMFLOAT3& v);

float Dot(const XMFLOAT3& a, const XMFLOAT3& b);

XMFLOAT3 Cross(const XMFLOAT3& a, const XMFLOAT3& b);

bool Equals(XMFLOAT3 a, XMFLOAT3 b);

bool Equals(XMFLOAT2 a, XMFLOAT2 b);

std::string ToString(const XMFLOAT3& v);

bool NextCharactersMatch(std::ifstream& file, const std::string& expected, bool resetPos);

bool XOR(bool a, bool b);

bool Approx(float a, float b);

float Rand01();

float Rand(float min, float max);

uint32_t PackColor(XMFLOAT4 color);

uint32_t PackColor(XMFLOAT3 color);

XMFLOAT4 UnpackColor4(uint32_t color);

XMFLOAT3 UnpackColor3(uint32_t color);