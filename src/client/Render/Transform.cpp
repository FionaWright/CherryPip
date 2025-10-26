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

void Transform::Rotate(const XMFLOAT3& rotation)
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

XMMATRIX Transform::GetModelMatrix() const
{
    const XMMATRIX T = XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
    const XMMATRIX R = XMMatrixRotationRollPitchYaw(m_rotationEuler.x, m_rotationEuler.y, m_rotationEuler.z);
    const XMMATRIX S = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);

    return XMMatrixMultiply(S, XMMatrixMultiply(R, T));
}