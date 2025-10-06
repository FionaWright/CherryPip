//
// Created by fiona on 06/10/2025.
//

#ifndef PT_OBJECT_H
#define PT_OBJECT_H
#include <memory>

#include "HWI/Material.h"
#include "HWI/Model.h"
#include "HWI/RootSig.h"
#include "HWI/Shader.h"
#include "HWI/Texture.h"
#include "Render/Transform.h"


struct CbvMatrices;

class Object
{
public:
    void Init(const std::shared_ptr<Transform>& transform,
              const std::shared_ptr<Shader>& shader, const std::shared_ptr<RootSig>& rootSig,
              const std::shared_ptr<Model>& model, const std::shared_ptr<Material>& mat);
    void SetParent(Object* parent);

    void Render(ID3D12GraphicsCommandList* cmdList, CbvMatrices& matrices) const;

    Transform* GetTransform() const { return m_transform.get(); }
    Shader* GetShader() const { return m_shader.get(); }
    RootSig* GetRootSig() const { return m_rootSig.get(); }
    Model* GetModel() const { return m_model.get(); }
    Object* GetParent() const { return m_parent; }
    Material* GetMaterial() const { return m_material.get(); }

private:
    Object* m_parent = nullptr;
    std::shared_ptr<Transform> m_transform;
    std::shared_ptr<RootSig> m_rootSig;
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<Model> m_model;
    std::shared_ptr<Material> m_material;
};


#endif //PT_OBJECT_H
