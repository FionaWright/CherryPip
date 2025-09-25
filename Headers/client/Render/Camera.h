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

private:
    XMFLOAT3 m_up = {}, m_forward = {}, m_right = {};
    XMFLOAT3 m_pos = XMFLOAT3(0,0,0);
    double m_pitch = 0.0, m_yaw = 0.0;
};


#endif //PT_CAMERA_H