//
// Created by fiona on 25/09/2025.
//

#include "Render/Camera.h"

void Camera::Init(XMFLOAT3 pos, XMFLOAT3 lookat)
{
    m_up = XMFLOAT3(0.0f, 1.0f, 0.0f);
    m_forward = XMFLOAT3(0.0f, 0.0f, -1.0f);
    m_right = XMFLOAT3(1.0f, 0.0f, 0.0f);

    m_yaw = atan2(m_forward.x, m_forward.z); // rotate around Y
    m_pitch = atan2(-m_forward.y, sqrt(m_forward.x * m_forward.x + m_forward.z * m_forward.z)); // rotate around X

    m_pos = XMFLOAT3(0, 0, 5);
}

XMMATRIX Camera::GetViewMatrix() const
{
    const XMVECTOR up = XMLoadFloat3(&m_up);
    const XMVECTOR dir = XMLoadFloat3(&m_forward);

    const XMVECTOR positionVector = XMLoadFloat3(&m_pos);

    return XMMatrixLookToLH(positionVector, dir, up);
}
