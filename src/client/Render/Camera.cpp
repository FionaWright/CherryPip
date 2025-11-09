//
// Created by fiona on 25/09/2025.
//

#include "Render/Camera.h"

#include "MathUtils.h"

void Camera::Init(const XMFLOAT3 pos, const float pitch, const float yaw)
{
    m_pitch = pitch;
    m_yaw = yaw;

    RecomputeUpRightForward();

    m_pos = pos;
}

XMMATRIX Camera::GetViewMatrix() const
{
    const XMVECTOR up = XMLoadFloat3(&m_up);
    const XMVECTOR dir = XMLoadFloat3(&m_forward);

    const XMVECTOR positionVector = XMLoadFloat3(&m_pos);

    return XMMatrixLookToLH(positionVector, dir, up);
}

void Camera::RecomputePitchYaw()
{
    m_yaw = atan2(m_forward.x, m_forward.z); // rotate around Y
    m_pitch = atan2(-m_forward.y, sqrt(m_forward.x * m_forward.x + m_forward.z * m_forward.z)); // rotate around X
}

void Camera::RecomputeUpRightForward()
{
    // horizontal radius
    const float r = cosf(m_pitch);

    m_forward = XMFLOAT3(
        sinf(m_yaw) * r,   // x
        -sinf(m_pitch),    // y  <-- negative to match RecomputePitchYaw()
        cosf(m_yaw) * r    // z
    );

    // normalize for safety (in case of rounding)
    m_forward = Normalize(m_forward);

    // keep the same cross order you used before so the handedness matches
    m_right = Normalize(Cross(m_forward, XMFLOAT3(0.0f, 1.0f, 0.0f)));
    m_up    = Cross(m_right, m_forward);
}
