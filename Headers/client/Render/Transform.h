//
// Created by fiona on 25/09/2025.
//

#ifndef PT_TRANSFORM_H
#define PT_TRANSFORM_H
#include <DirectXMath.h>

using namespace DirectX;

class Transform
{
public:
    const XMFLOAT3& GetPosition() const { return m_position; }
    const XMFLOAT3& GetRotation() const { return m_rotationEuler; }
    const XMFLOAT3& GetScale() const { return m_scale; }

    void SetPosition(const XMFLOAT3& position) { m_position = position; }
    void SetPosition(const float x, const float y, const float z) { m_position = {x, y, z}; }
    void SetRotation(const XMFLOAT3& rotation) { m_rotationEuler = rotation; }
    void SetRotation(const float x, const float y, const float z) { m_rotationEuler = {x, y, z}; }
    void SetScale(const XMFLOAT3& scale) { m_scale = scale; }
    void SetScale(const float x, const float y, const float z) { m_scale = {x, y, z}; }

    void Translate(const XMFLOAT3& offset);
    void Rotate(const XMFLOAT3& rotation);
    void Scale(const XMFLOAT3& scale);

    XMMATRIX GetModelMatrix() const;

private:
    XMFLOAT3 m_position = XMFLOAT3(0, 0, 0);
    XMFLOAT3 m_rotationEuler = XMFLOAT3(0, 0, 0);
    XMFLOAT3 m_scale = XMFLOAT3(1, 1, 1);
};


#endif //PT_TRANSFORM_H
