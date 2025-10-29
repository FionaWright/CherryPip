//
// Created by fionaw on 25/09/2025.
//

#ifndef PT_CAMERACONTROLLER_H
#define PT_CAMERACONTROLLER_H
#include "Camera.h"


class CameraController
{
public:
    void Init(const XMFLOAT3& pos, const XMFLOAT3& lookat) { m_camera.Init(pos, lookat); }
    bool UpdateCamera();

    XMMATRIX GetViewMatrix() const { return m_camera.GetViewMatrix(); }
    Camera& GetCamera() { return m_camera; }
    const Camera& GetCamera() const { return m_camera; }

private:
    Camera m_camera;
    float m_speed = 0.0225f;
    float m_rotationSpeed = 0.015f;
};

#endif //PT_CAMERACONTROLLER_H