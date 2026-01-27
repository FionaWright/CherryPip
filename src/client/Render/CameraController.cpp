//
// Created by fionaw on 25/09/2025.
//

#include "System/pch.h"
#include "Render/CameraController.h"
#include "System/Config.h"
#include "System/Input.h"

bool CameraController::UpdateCamera(double deltaTime)
{
    const bool mouseOverGUI = Input::GetMousePos().x < Config::GetSystem().WindowAppGuiWidth || Input::GetMousePos().x > Config::GetSystem().WindowAppGuiWidth + Config::GetSystem().RtvWidth;

    if (Input::IsMouseRight() && !mouseOverGUI)
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
        return deltaMouse.x != 0 || deltaMouse.y != 0;
    }

    const XMFLOAT3 up = m_camera.GetUp();
    const XMFLOAT3 right = m_camera.GetRight();

    if (Input::IsMouseMiddle() && !mouseOverGUI)
    {
        XMFLOAT2 deltaMouse = Input::GetMousePosDelta();
        const float panSpeed = 3.5f * m_speed;
        deltaMouse.x *= panSpeed;
        deltaMouse.y *= -panSpeed;

        const XMFLOAT3 upTranslation = XMFLOAT3(up.x * -deltaMouse.y, up.y * -deltaMouse.y, up.z * -deltaMouse.y);
        m_camera.AddPosition(upTranslation);

        const XMFLOAT3 rightTranslation = XMFLOAT3(right.x * deltaMouse.x, right.y * deltaMouse.x, right.z * deltaMouse.x);
        m_camera.AddPosition(rightTranslation);
        return deltaMouse.x != 0 || deltaMouse.y != 0;
    }

    const XMFLOAT3 forward = m_camera.GetForward();

    float forwardScalar = 0.0f;
    if (!mouseOverGUI)
        forwardScalar = Input::GetMouseWheelDelta() * 230 * m_speed;
    if (Input::IsKey(KeyCode::W))
    {
        forwardScalar += m_speed * deltaTime;
    }
    else if (Input::IsKey(KeyCode::S))
    {
        forwardScalar -= m_speed * deltaTime;
    }

    const XMFLOAT3 forwardTranslation = XMFLOAT3(forward.x * forwardScalar, forward.y * forwardScalar, forward.z * forwardScalar);
    m_camera.AddPosition(forwardTranslation);

    float rightScalar = 0;
    if (Input::IsKey(KeyCode::A))
    {
        rightScalar += m_speed * deltaTime;
    }
    else if (Input::IsKey(KeyCode::D))
    {
        rightScalar -= m_speed * deltaTime;
    }

    const XMFLOAT3 rightTranslation = XMFLOAT3(right.x * rightScalar, right.y * rightScalar, right.z * rightScalar);
    m_camera.AddPosition(rightTranslation);

    float upScalar = 0;
    if (Input::IsKey(KeyCode::E))
    {
        upScalar += m_speed * deltaTime;
    }
    else if (Input::IsKey(KeyCode::Q))
    {
        upScalar -= m_speed * deltaTime;
    }

    const XMFLOAT3 upTranslation = XMFLOAT3(up.x * upScalar, up.y * upScalar, up.z * upScalar);
    m_camera.AddPosition(upTranslation);

    return forwardScalar != 0 || rightScalar != 0 || upScalar != 0;
}
