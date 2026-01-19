//
// Created by fionaw on 09/11/2025.
//

#ifndef CHERRYPIP_SCENE_H
#define CHERRYPIP_SCENE_H


#include "HWI/Material.h"
#include "HWI/Model.h"

class Object;

class Scene
{
public:
    void Init(const char* name, const XMFLOAT3& camPos, float pitch, float yaw, const std::vector<std::shared_ptr<Object>>& objects);
    void Init(const char* name, const XMFLOAT3& camPos, float pitch, float yaw,const std::shared_ptr<Object>& object);
    void Init(const char* name, const XMFLOAT3& camPos, float pitch, float yaw,const std::shared_ptr<Model>& model, const MaterialData& materialData = {});

    const char* GetName() const { return m_name.c_str(); }
    const std::vector<std::shared_ptr<Object>>& GetObjects() const { return m_objects; }
    XMFLOAT3 GetCameraPosition() const { return m_cameraPosition; }
    float GetPitch() const { return m_cameraPitch; }
    float GetYaw() const { return m_cameraYaw; }

private:
    std::string m_name;
    std::vector<std::shared_ptr<Object>> m_objects;
    XMFLOAT3 m_cameraPosition = {};
    float m_cameraPitch = 0.0f;
    float m_cameraYaw = 0.0f;
};


#endif //CHERRYPIP_SCENE_H
