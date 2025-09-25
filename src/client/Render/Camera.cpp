//
// Created by fiona on 25/09/2025.
//

#include "Render/Camera.h"

void Camera::Init(XMFLOAT3 pos, XMFLOAT3 lookat)
{
    m_up = XMFLOAT3(0.0f, 1.0f, 0.0f);
    m_forward = XMFLOAT3(0.0f, 0.0f, -1.0f);
    m_right = XMFLOAT3(1.0f, 0.0f, 0.0f);

    m_pos = XMFLOAT3(0, 0, 5);
}

XMMATRIX Camera::GetViewMatrix() const
{
    XMVECTOR up = XMLoadFloat3(&m_up);
    XMVECTOR dir = XMLoadFloat3(&m_forward);

    XMVECTOR positionVector = XMLoadFloat3(&m_pos);

    return XMMatrixLookToLH(positionVector, dir, up);
}
