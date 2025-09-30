//
// Created by fionaw on 25/09/2025.
//

#include "Render/CameraController.h"

#include "System/Input.h"

void CameraController::UpdateCamera()
{
    if (Input::IsMouseRight())
    {
        double pitch = m_camera.GetPitch();
        double yaw = m_camera.GetYaw();

        XMFLOAT2 deltaMouse = Input::GetMousePosDelta();
        deltaMouse.x *= m_rotationSpeed;
        deltaMouse.y *= m_rotationSpeed;

        pitch += deltaMouse.y;
        yaw += deltaMouse.x;
        m_camera.SetPitch(pitch);
        m_camera.SetYaw(yaw);

        XMVECTOR rot = XMQuaternionRotationRollPitchYaw(pitch, yaw, 0);
        rot = XMQuaternionNormalize(rot);

        const XMVECTOR direction = XMVector3Rotate(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rot);
        m_camera.SetForward(direction);

        const XMVECTOR up = XMVector3Rotate(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), rot);
        m_camera.SetUp(up);

        const XMVECTOR right = XMVector3Rotate(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), rot);
        m_camera.SetRight(right);
        return;
    }

    const XMFLOAT3 up = m_camera.GetUp();
    const XMFLOAT3 right = m_camera.GetRight();

    if (Input::IsMouseMiddle())
    {
        XMFLOAT2 deltaMouse = Input::GetMousePosDelta();
        const float panSpeed = -3.5f * m_speed;
        deltaMouse.x *= panSpeed;
        deltaMouse.y *= panSpeed;

        const XMFLOAT3 upTranslation = XMFLOAT3(up.x * -deltaMouse.y, up.y * -deltaMouse.y, up.z * -deltaMouse.y);
        m_camera.AddPosition(upTranslation);

        const XMFLOAT3 rightTranslation = XMFLOAT3(right.x * deltaMouse.x, right.y * deltaMouse.x, right.z * deltaMouse.x);
        m_camera.AddPosition(rightTranslation);
        return;
    }

    const XMFLOAT3 forward = m_camera.GetForward();

    float forwardScalar = Input::GetMouseWheelDelta();
    forwardScalar *= 230 * m_speed;
    if (Input::IsKey(KeyCode::W))
    {
        forwardScalar += m_speed;
    }
    else if (Input::IsKey(KeyCode::S))
    {
        forwardScalar -= m_speed;
    }

    const XMFLOAT3 forwardTranslation = XMFLOAT3(forward.x * forwardScalar, forward.y * forwardScalar, forward.z * forwardScalar);
    m_camera.AddPosition(forwardTranslation);

    float rightScalar = 0;
    if (Input::IsKey(KeyCode::A))
    {
        rightScalar += m_speed;
    }
    else if (Input::IsKey(KeyCode::D))
    {
        rightScalar -= m_speed;
    }

    const XMFLOAT3 rightTranslation = XMFLOAT3(right.x * rightScalar, right.y * rightScalar, right.z * rightScalar);
    m_camera.AddPosition(rightTranslation);

    float upScalar = 0;
    if (Input::IsKey(KeyCode::E))
    {
        upScalar += m_speed;
    }
    else if (Input::IsKey(KeyCode::Q))
    {
        upScalar -= m_speed;
    }

    const XMFLOAT3 upTranslation = XMFLOAT3(up.x * upScalar, up.y * upScalar, up.z * upScalar);
    m_camera.AddPosition(upTranslation);
}
