//
// Created by fionaw on 09/11/2025.
//


#include "Render/Scene.h"
#include "HWI/Material.h"
#include "Render/Object.h"

void Scene::Init(const char* name, const XMFLOAT3& camPos, float pitch, float yaw,const std::vector<std::shared_ptr<Object>>& objects)
{
    m_name = std::string(name);
    m_objects = objects;
    m_cameraPitch = pitch;
    m_cameraYaw = yaw;
    m_cameraPosition = camPos;
}

void Scene::Init(const char* name, const XMFLOAT3& camPos, float pitch, float yaw,const std::shared_ptr<Object>& object)
{
    m_name = std::string(name);
    m_objects.push_back(object);
    m_cameraPitch = pitch;
    m_cameraYaw = yaw;
    m_cameraPosition = camPos;
}

void Scene::Init(const char* name, const XMFLOAT3& camPos, const float pitch, const float yaw, const std::shared_ptr<Model>& model, const MaterialData& materialData)
{
    m_name = std::string(name);
    const auto material = std::make_shared<Material>();
    material->SetData(materialData);

    const auto object = std::make_shared<Object>();
    object->Init(name, nullptr, nullptr, nullptr, model, material, nullptr);
    m_objects.push_back(object);

    m_cameraPitch = pitch;
    m_cameraYaw = yaw;
    m_cameraPosition = camPos;
}
