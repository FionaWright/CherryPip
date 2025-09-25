//
// Created by fiona on 25/09/2025.
//

#ifndef PT_CAMERA_H
#define PT_CAMERA_H
#include <DirectXMath.h>

using namespace DirectX;

class Camera
{
public:
    void Init(XMFLOAT3 pos, XMFLOAT3 lookat);
    XMMATRIX GetViewMatrix() const;

    double GetPitch() const { return m_pitch; }
    double GetYaw() const { return m_yaw; }

    XMFLOAT3 GetPosition() const { return m_pos; }
    XMFLOAT3 GetUp() const { return m_up; }
    XMFLOAT3 GetRight() const { return m_right; }
    XMFLOAT3 GetForward() const { return m_forward; }

    void SetPitch(const double pitch) { m_pitch = pitch; }
    void SetYaw(const double yaw) { m_yaw = yaw; }

    void SetPosition(const XMFLOAT3& pos) { m_pos = pos; }
    void AddPosition(const XMFLOAT3 offset) { m_pos.x += offset.x; m_pos.y += offset.y; m_pos.z += offset.z; }

    void SetUp(const XMFLOAT3& up) { m_up = up; }
    void SetRight(const XMFLOAT3& right) { m_right = right; }
    void SetForward(const XMFLOAT3& forward) { m_forward = forward; }
    void SetUp(const XMVECTOR& up) { XMStoreFloat3(&m_up, up); }
    void SetRight(const XMVECTOR& right) { XMStoreFloat3(&m_right, right); }
    void SetForward(const XMVECTOR& forward) { XMStoreFloat3(&m_forward, forward); }

private:
    XMFLOAT3 m_up = {}, m_forward = {}, m_right = {};
    XMFLOAT3 m_pos = XMFLOAT3(0,0,0);
    double m_pitch = 0.0, m_yaw = 0.0;
};


#endif //PT_CAMERA_H