//
// Created by fionaw on 09/11/2025.
//

#include "Render/Scene.h"

#include "HWI/Material.h"
#include "Render/Object.h"

void Scene::Init(const char* name, const std::vector<std::shared_ptr<Object>>& objects)
{
    m_name = std::string(name);
    m_objects = objects;
}

void Scene::Init(const char* name, const std::shared_ptr<Object>& object)
{
    m_name = std::string(name);
    m_objects.push_back(object);
}

void Scene::Init(const char* name, const std::shared_ptr<Model>& model, const MaterialData& materialData)
{
    m_name = std::string(name);
    const auto material = std::make_shared<Material>();
    material->SetData(materialData);

    const auto object = std::make_shared<Object>();
    object->Init(nullptr, nullptr, nullptr, model, material);
    m_objects.push_back(object);
}
