//
// Created by fiona on 25/09/2025.
//

#ifndef PT_TRANSFORM_H
#define PT_TRANSFORM_H

class Transform
{
public:
    [[nodiscard]] const XMFLOAT3& GetPosition() const { return m_position; }
    [[nodiscard]] const XMFLOAT3& GetRotationE() const { return m_rotationEuler; }
    [[nodiscard]] const XMVECTOR& GetRotationQ() const { return m_rotationQ; }
    [[nodiscard]] const XMFLOAT3& GetScale() const { return m_scale; }

    void SetPosition(const XMFLOAT3& position) { m_position = position; }
    void SetPosition(const float x, const float y, const float z) { m_position = {x, y, z}; }

    void SetScale(const XMFLOAT3& scale) { m_scale = scale; }
    void SetScale(const float x, const float y, const float z) { m_scale = {x, y, z}; }
    void SetScale(const float x) { m_scale = {x, x, x}; }

    void SetRotationE(const XMFLOAT3& rotation) { m_rotationEuler = rotation; rotationEtoQ(); }
    void SetRotationE(const float x, const float y, const float z) { m_rotationEuler = {x, y, z}; rotationEtoQ(); }
    void SetRotationQ(const XMVECTOR& rotation) { m_rotationQ = rotation; rotationQtoE(); }

    void Translate(const XMFLOAT3& offset);
    void RotateE(const XMFLOAT3& rotation);
    void Scale(const XMFLOAT3& scale);

    [[nodiscard]] XMMATRIX GetModelMatrix(XMFLOAT3 centroid = {}) const;

private:
    void rotationEtoQ() { m_rotationQ = XMQuaternionRotationRollPitchYaw(m_rotationEuler.x, m_rotationEuler.y, m_rotationEuler.z); }
    void rotationQtoE();

    XMFLOAT3 m_position = XMFLOAT3(0, 0, 0);
    XMFLOAT3 m_rotationEuler = XMFLOAT3(0, 0, 0);
    XMVECTOR m_rotationQ = XMQuaternionIdentity();
    XMFLOAT3 m_scale = XMFLOAT3(1, 1, 1);
};


#endif //PT_TRANSFORM_H
