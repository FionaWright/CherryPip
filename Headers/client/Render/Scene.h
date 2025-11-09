//
// Created by fionaw on 09/11/2025.
//

#ifndef CHERRYPIP_SCENE_H
#define CHERRYPIP_SCENE_H
#include <memory>
#include <vector>

#include "HWI/Material.h"
#include "HWI/Model.h"


class Object;

class Scene
{
public:
    void Init(const char* name, const std::vector<std::shared_ptr<Object>>& objects);
    void Init(const char* name, const std::shared_ptr<Object>& object);
    void Init(const char* name, const std::shared_ptr<Model>& model, const MaterialData& materialData = {});

    const char* GetName() const { return m_name.c_str(); }
    const std::vector<std::shared_ptr<Object>>& GetObjects() const { return m_objects; }

private:
    std::string m_name;
    std::vector<std::shared_ptr<Object>> m_objects;
};


#endif //CHERRYPIP_SCENE_H
