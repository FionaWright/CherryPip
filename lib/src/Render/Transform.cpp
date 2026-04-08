//
// Created by fiona on 25/09/2025.
//



#include "Render/Transform.h"

void Transform::Translate(const XMFLOAT3& offset)
{
    m_position.x += offset.x;
    m_position.y += offset.y;
    m_position.z += offset.z;
}

void Transform::RotateE(const XMFLOAT3& rotation)
{
    m_rotationEuler.x += rotation.x;
    m_rotationEuler.y += rotation.y;
    m_rotationEuler.z += rotation.z;
}

void Transform::Scale(const XMFLOAT3& scale)
{
    m_scale.x *= scale.x;
    m_scale.y *= scale.y;
    m_scale.z *= scale.z;
}

XMMATRIX Transform::GetModelMatrix(const XMFLOAT3 centroid) const
{
    const XMMATRIX T = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
    const XMMATRIX R = XMMatrixRotationQuaternion(m_rotationQ);
    const XMMATRIX S = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

    assert(centroid.x == 0.0f); // Disabled for now
    if (centroid.x == 0.0f)
        return XMMatrixMultiply(S, XMMatrixMultiply(R, T));

    const XMMATRIX C = XMMatrixTranslation(centroid.x, centroid.y, centroid.z);
    const XMMATRIX CI = XMMatrixInverse(nullptr, C);
    return XMMatrixMultiply(CI, XMMatrixMultiply(S, XMMatrixMultiply(R, XMMatrixMultiply(T, C))));
}

void Transform::rotationQtoE() // For GUI purposes only
{
    const XMMATRIX rotMatrix = XMMatrixRotationQuaternion(m_rotationQ);
    m_rotationEuler.x = std::asin(-rotMatrix.r[2].m128_f32[1]); // X-axis rotation
    m_rotationEuler.y = std::atan2(rotMatrix.r[2].m128_f32[0], rotMatrix.r[2].m128_f32[2]); // Y-axis
    m_rotationEuler.z = std::atan2(rotMatrix.r[0].m128_f32[1], rotMatrix.r[1].m128_f32[1]); // Z-axis
}
