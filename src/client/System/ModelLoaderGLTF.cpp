#include "System/ModelLoaderGLTF.h"
#include <direct.h>
#include <filesystem>
#include <iostream>
#include <ostream>

#include "Helper.h"
#include "HWI/Model.h"
#include "MathUtils.h"
#include "HWI/D3D.h"
#include "HWI/Texture.h"

#include "CBV.h"
#include "Debug/Profiler.h"
#include "HWI/BLAS.h"
#include "HWI/Heap.h"
#include "HWI/Material.h"
#include "Render/Object.h"
#include "System/FileHelper.h"
#include "System/TextureLoader.h"

namespace filesystem = std::filesystem;

fastgltf::Parser ModelLoaderGLTF::ms_parser;
bool ModelLoaderGLTF::ms_initialisedParser;
std::mutex ModelLoaderGLTF::ms_batchAddMutex;

// TODO: This is unideal, consider redoing everything

Transform ModelLoaderGLTF::toTransform(fastgltf::TRS& trs)
{
    Transform transform;

    auto& pos = trs.translation;
    transform.SetPosition(pos.x(), pos.y(), pos.z());

    auto& rot = trs.rotation;
    const XMFLOAT4 rotFloat4 = XMFLOAT4(rot.x(), rot.y(), rot.z(), rot.w());
    const XMVECTOR rotVec = XMLoadFloat4(&rotFloat4);
    transform.SetRotationQ(rotVec);
    auto& scale = trs.scale;
    transform.SetScale(scale.x(), scale.y(), scale.z());

    return transform;
}

template <typename Func>
void loadGLTFVertexData(const std::string& directory, std::vector<VertexInputDataGLTF>& vBuffer, Asset& asset,
                        const fastgltf::Primitive& primitive, const char* attribute, Func func)
{
    const fastgltf::Attribute* attributeObj = primitive.findAttribute(attribute);
    if (attributeObj == primitive.attributes.cend())
    {
        return;
    }

    const auto& accessor = (*asset)->accessors.at(attributeObj->accessorIndex);
    const auto& bufferView = (*asset)->bufferViews.at(*accessor.bufferViewIndex);
    const auto& bufferData = (*asset)->buffers.at(bufferView.bufferIndex).data;

    const size_t dataOffset = bufferView.byteOffset + accessor.byteOffset;
    const size_t byteSize = fastgltf::getElementByteSize(accessor.type, accessor.componentType);
    const size_t dataStride = bufferView.byteStride.value_or(byteSize);

    const std::byte* pData = nullptr;
    std::vector<std::byte> pTempFileData;

    if (bufferData.index() == 3)
        pData = std::get<fastgltf::sources::Array>(bufferData).bytes.data() + dataOffset;
    else if (bufferData.index() == 2)
    {
        auto& uri = std::get<fastgltf::sources::URI>(bufferData);
        std::string path(uri.uri.path());
        std::ifstream file(directory + path, std::ios::binary);

        assert(file.is_open());

        file.seekg(dataOffset + uri.fileByteOffset, std::ios::beg);

        size_t totalBytes = accessor.count * byteSize;
        pTempFileData.resize(totalBytes);
        file.read(reinterpret_cast<char*>(pTempFileData.data()), totalBytes);
        file.close();

        pData = pTempFileData.data();

        if (!file)
            throw std::runtime_error("Failed to read the required data from file.");
    }
    else
        throw new std::exception("Invalid buffer data type");

    if (vBuffer.size() < accessor.count)
        vBuffer.resize(accessor.count);

    for (size_t i = 0; i < vBuffer.size(); ++i)
    {
        const std::byte* address = pData + i * dataStride;
        func(address, &vBuffer[i]);
    }
}

void ModelLoaderGLTF::loadGLTFIndices(const std::string& directory, std::vector<uint32_t>& iBuffer, Asset& asset,
                                      const fastgltf::Primitive& primitive, bool convertRhToLh)
{
    const auto& accessor = (*asset)->accessors[primitive.indicesAccessor.value()];
    const auto& bufferView = (*asset)->bufferViews[*accessor.bufferViewIndex];
    const auto& bufferData = (*asset)->buffers[bufferView.bufferIndex].data;

    const size_t dataOffset = bufferView.byteOffset + accessor.byteOffset;
    const size_t indexByteSize = fastgltf::getElementByteSize(accessor.type, accessor.componentType);
    const size_t dataStride = bufferView.byteStride.value_or(indexByteSize);

    const std::byte* pData = nullptr;
    std::vector<std::byte> pTempFileData;

    if (bufferData.index() == 3)
        pData = std::get<fastgltf::sources::Array>(bufferData).bytes.data() + dataOffset;
    else if (bufferData.index() == 2)
    {
        auto& uri = std::get<fastgltf::sources::URI>(bufferData);
        const std::string path(uri.uri.path());
        std::ifstream file(directory + path, std::ios::binary);

        if (!file.is_open())
            throw std::runtime_error("Failed to open file");

        file.seekg(dataOffset + uri.fileByteOffset, std::ios::beg);

        const size_t totalBytes = accessor.count * indexByteSize;
        pTempFileData.resize(totalBytes);
        file.read(reinterpret_cast<char*>(pTempFileData.data()), totalBytes);
        file.close();

        pData = pTempFileData.data();

        if (!file)
            throw std::runtime_error("Failed to read the required data from file.");
    }
    else
        throw std::exception("Invalid buffer data type");

    iBuffer.resize(accessor.count);

    if (!convertRhToLh && indexByteSize == 4)
    {
        std::memcpy(iBuffer.data(), pData, accessor.count * 4);
        return;
    }

    for (size_t i = 0; i < iBuffer.size(); i += 3)
    {
        const std::byte* indexData0 = pData + (i + 0) * dataStride;
        const std::byte* const indexData1 = pData + (i + 1) * dataStride;
        const std::byte* const indexData2 = pData + (i + 2) * dataStride;

        switch (indexByteSize)
        {
        case 1:
            if (convertRhToLh)
            {
                iBuffer[i + 2] = static_cast<uint8_t>(*indexData0);
                iBuffer[i + 1] = static_cast<uint8_t>(*indexData1);
                iBuffer[i + 0] = static_cast<uint8_t>(*indexData2);
                break;
            }

            iBuffer[i + 0] = static_cast<uint8_t>(*indexData0);
            iBuffer[i + 1] = static_cast<uint8_t>(*indexData1);
            iBuffer[i + 2] = static_cast<uint8_t>(*indexData2);
            break;

        case 2:
            uint16_t value16;

            if (convertRhToLh)
            {
                std::memcpy(&value16, indexData0, sizeof(uint16_t));
                iBuffer[i + 2] = value16;
                std::memcpy(&value16, indexData1, sizeof(uint16_t));
                iBuffer[i + 1] = value16;
                std::memcpy(&value16, indexData2, sizeof(uint16_t));
                iBuffer[i + 0] = value16;
                break;
            }

            std::memcpy(&value16, indexData0, sizeof(uint16_t));
            iBuffer[i + 0] = value16;
            std::memcpy(&value16, indexData1, sizeof(uint16_t));
            iBuffer[i + 1] = value16;
            std::memcpy(&value16, indexData2, sizeof(uint16_t));
            iBuffer[i + 2] = value16;
            break;

        case 4: // Only reached when converting RHS -> LHS
            std::memcpy(&iBuffer[i + 2], indexData0, sizeof(uint32_t));
            std::memcpy(&iBuffer[i + 1], indexData1, sizeof(uint32_t));
            std::memcpy(&iBuffer[i + 0], indexData2, sizeof(uint32_t));
            break;

        default:
            throw std::invalid_argument("Error: Unexpected indices data size (" + std::to_string(indexByteSize) + ").");
        }
    }
}

void ModelLoaderGLTF::loadModel(const D3D* d3d, ID3D12GraphicsCommandList* cmdList, const std::string& directory,
                                Asset& asset, const fastgltf::Primitive& primitive, Model* model, bool convertRhToLh)
{
    Profiler::AddToStack(directory.c_str());

    std::vector<VertexInputDataGLTF> vertexBuffer;

    loadGLTFVertexData(directory, vertexBuffer, asset, primitive, "POSITION",
                       [convertRhToLh](const std::byte* address, VertexInputDataGLTF* output)
                       {
                           output->Position = *reinterpret_cast<const XMFLOAT3*>(address);
                           if (convertRhToLh)
                               output->Position.x = -output->Position.x;
                       });

    const size_t vertexCount = vertexBuffer.size();
    if (vertexCount == 0)
        return;

    loadGLTFVertexData(directory, vertexBuffer, asset, primitive, "TEXCOORD_0",
                       [](const std::byte* address, VertexInputDataGLTF* output)
                       {
                           output->Texture = *reinterpret_cast<const XMFLOAT2*>(address);
                       });

    loadGLTFVertexData(directory, vertexBuffer, asset, primitive, "NORMAL",
                       [convertRhToLh](const std::byte* address, VertexInputDataGLTF* output)
                       {
                           output->Normal = Normalize(*reinterpret_cast<const XMFLOAT3*>(address));
                           if (convertRhToLh)
                               output->Normal.x = -output->Normal.x;
                       });

    loadGLTFVertexData(directory, vertexBuffer, asset, primitive, "TANGENT",
                       [convertRhToLh](const std::byte* address, VertexInputDataGLTF* output)
                       {
                           const auto* data = reinterpret_cast<const XMFLOAT4*>(address);
                           const float handedness = data->w > 0.0f ? 1.0f : -1.0f;
                           output->Tangent = Normalize(Mult(XMFLOAT3(data->x, data->y, data->z), handedness));
                           if (convertRhToLh)
                               output->Tangent.x = -output->Tangent.x;
                       });

    float boundingRadiusSq = 0;
    struct Double3
    {
        double X = 0, Y = 0, Z = 0;
    } rollingCentroidSum;

    for (size_t j = 0; j < vertexCount; j++)
    {
        vertexBuffer[j].Binormal = Normalize(Cross(vertexBuffer[j].Tangent, vertexBuffer[j].Normal));

        rollingCentroidSum.X += vertexBuffer[j].Position.x;
        rollingCentroidSum.Y += vertexBuffer[j].Position.y;
        rollingCentroidSum.Z += vertexBuffer[j].Position.z;
    }

    rollingCentroidSum.X /= vertexCount;
    rollingCentroidSum.Y /= vertexCount;
    rollingCentroidSum.Z /= vertexCount;
    const XMFLOAT3 centroidFloat3 = XMFLOAT3(static_cast<float>(rollingCentroidSum.X),
                                       static_cast<float>(rollingCentroidSum.Y),
                                       static_cast<float>(rollingCentroidSum.Z));

    for (size_t j = 0; j < vertexCount; j++)
    {
        XMFLOAT3 diff = Subtract(centroidFloat3, vertexBuffer[j].Position);
        float magSq = Dot(diff, diff);
        if (magSq > boundingRadiusSq)
            boundingRadiusSq = magSq;
    }

    boundingRadiusSq = std::sqrt(boundingRadiusSq);

    std::vector<uint32_t> indexBuffer;
    loadGLTFIndices(directory, indexBuffer, asset, primitive, convertRhToLh);

    model->Init(d3d->GetDevice(), vertexBuffer.size(), indexBuffer.size(), sizeof(VertexInputDataGLTF),
                boundingRadiusSq, centroidFloat3);
    model->SetBuffers(d3d->GetDevice(), cmdList, vertexBuffer.data(), indexBuffer.data());

    Profiler::PopAndPrint();
}

std::variant<std::string, const std::byte*> ModelLoaderGLTF::loadTexture(
    const Asset& asset, const size_t textureIndex, size_t& outDataSize)
{
    fastgltf::Texture& tex = (*asset)->textures[textureIndex];
    fastgltf::Image& image = (*asset)->images[tex.imageIndex.value()];

    if (image.data.index() == 2)
    {
        std::string texName(std::get<fastgltf::sources::URI>(image.data).uri.path());
        const size_t slashIndex = texName.find_last_of('/');

        if (slashIndex != std::string::npos)
            texName = texName.substr(slashIndex + 1, texName.size() - slashIndex - 1);

        return texName;
    }

    if (image.data.index() == 1)
    {
        auto& bufferViewInfo = std::get<fastgltf::sources::BufferView>(image.data);
        if (bufferViewInfo.mimeType != fastgltf::MimeType::PNG)
            throw std::exception("GLB Mime type not supported");

        const auto& bufferView = (*asset)->bufferViews[bufferViewInfo.bufferViewIndex];
        const auto& bufferData = (*asset)->buffers[bufferView.bufferIndex].data;

        if (bufferData.index() != 3)
            throw std::exception("Not sure what to do with this");

        const std::byte* pData = std::get<fastgltf::sources::Array>(bufferData).bytes.data() + bufferView.byteOffset;
        outDataSize = bufferView.byteLength;
        return pData;
    }

    return "";
}

void ModelLoaderGLTF::loadPrimitive(D3D* d3d, ID3D12GraphicsCommandList* cmdList, Heap* heap, Asset& asset,
                                    const fastgltf::Primitive& primitive, const std::string& modelNameExtensionless,
                                    fastgltf::Node& node, GLTFLoadArgs& args, Transform transform, std::string id, size_t meshIndex,
                                    size_t primitiveIndex)
{
    const uint32_t slashIdx = modelNameExtensionless.find_last_of('/');
    const std::string assetDirectory = wstringToString(FileHelper::GetAssetsPath());
    const std::string localDirectory = assetDirectory + "Models/" + modelNameExtensionless.substr(0, slashIdx) + "/";

    std::shared_ptr<Model> model = std::make_shared<Model>();
    loadModel(d3d, cmdList, localDirectory, asset, primitive, model.get(), args.ConvertRhToLh);

    UINT shaderIndex = -1;

    for (auto & Override : args.Overrides)
    {
        if (Override.WhiteList.empty())
            continue;

        bool found = false;
        for (const auto& str : Override.WhiteList)
        {
            if (node.name.starts_with(str))
            {
                found = true;
                break;
            }
        }
        if (!found)
            continue;

        shaderIndex = Override.ShaderIndex;
        break;
    }

    std::string nodeName(node.name);
    nodeName = id + "::" + nodeName;

    auto t = std::make_shared<Transform>();
    *t = transform;

    if (shaderIndex == -1)
        shaderIndex = args.DefaultShaderIndex;

    std::shared_ptr<Shader> shaderUsed = shaderIndex == -1 ? nullptr : args.Shaders[shaderIndex];

    // TODO: Refactor
    if ((*asset)->materials.empty())
    {
        std::string diffuseTexInput = assetDirectory + "Textures/TestTex.dds";
        std::shared_ptr<Texture> diffuseTex = std::make_shared<Texture>();
        diffuseTex->Init(d3d->GetDevice(), cmdList, diffuseTexInput,
                         1);

        std::shared_ptr<Material> material = std::make_shared<Material>();
        material->Init(heap);
        material->AddCBV(d3d->GetDevice(), heap, sizeof(CbvMatrices));
        material->AddCBV(d3d->GetDevice(), heap, sizeof(CbvRasterDebug));
        material->SetTex(d3d->GetDevice(), 0, heap, diffuseTex);

        MaterialData materialData = {};
        materialData.BindlessTexDiffuse = heap->AddBindlessTexture(d3d->GetDevice(), diffuseTex);
        material->SetData(materialData);

        auto obj = std::make_shared<Object>();
        obj->Init(node.name.c_str(), t, shaderUsed, args.Root, model, material);
        args.OutObjects.emplace_back(obj);
        return;
    }

    fastgltf::Material& mat = (*asset)->materials[primitive.materialIndex.value_or(0)];

    std::variant<std::string, const std::byte*> diffuseTexInput = "";
    size_t dataSize = 0;
    if (mat.pbrData.baseColorTexture.has_value())
    {
        diffuseTexInput = loadTexture(asset, mat.pbrData.baseColorTexture.value().textureIndex, dataSize);
        if (std::holds_alternative<std::string>(diffuseTexInput))
            diffuseTexInput = localDirectory + get<std::string>(diffuseTexInput);
    }
    else if (mat.iridescence)
        diffuseTexInput = assetDirectory + "Textures/Transparent.dds";
    else
        diffuseTexInput = assetDirectory + "Textures/WhitePOT.dds";

    std::shared_ptr<Texture> diffuseTex = std::make_shared<Texture>();
    if (std::holds_alternative<std::string>(diffuseTexInput))
    {
        auto texPath = get<std::string>(diffuseTexInput);
        const auto pngIdx = texPath.find("png");
        if (pngIdx != std::string::npos)
            texPath = texPath.replace(pngIdx, 3, "dds");
        diffuseTex->Init(d3d->GetDevice(), cmdList, texPath,
                         1);
    }
    else
    {
        auto pData = get<const std::byte*>(diffuseTexInput);
        diffuseTex->InitPNG(d3d->GetDevice(), cmdList, reinterpret_cast<const uint8_t*>(pData), dataSize,
                            DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    }

    std::variant<std::string, const std::byte*> normalTexInput = "";
    if (mat.normalTexture.has_value())
    {
        normalTexInput = loadTexture(asset, mat.normalTexture.value().textureIndex, dataSize);
        if (std::holds_alternative<std::string>(normalTexInput))
            normalTexInput = localDirectory + get<std::string>(normalTexInput);
    }
    else
        normalTexInput = "Assets/Textures/DefaultNormal.dds";

    std::shared_ptr<Texture> normalTex = std::make_shared<Texture>();
    if (std::holds_alternative<std::string>(normalTexInput))
    {
        auto texPath = get<std::string>(normalTexInput);
        const auto pngIdx = texPath.find("png");
        if (pngIdx != std::string::npos)
            texPath = texPath.replace(pngIdx, 3, "dds");
        normalTex->Init(d3d->GetDevice(), cmdList, texPath,
                         1);
    }
    else
    {
        auto pData = get<const std::byte*>(normalTexInput);
        normalTex->InitPNG(d3d->GetDevice(), cmdList, reinterpret_cast<const uint8_t*>(pData), dataSize,
                            DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    }

    std::variant<std::string, const std::byte*> roughMetTexInput = "";
    if (mat.pbrData.metallicRoughnessTexture.has_value())
    {
        roughMetTexInput = loadTexture(asset, mat.pbrData.metallicRoughnessTexture.value().textureIndex, dataSize);
        if (std::holds_alternative<std::string>(roughMetTexInput))
            roughMetTexInput = localDirectory + get<std::string>(roughMetTexInput);
    }
    else
        roughMetTexInput = "Assets/Textures/WhitePOT.dds";

    std::shared_ptr<Texture> roughMetTex = std::make_shared<Texture>();
    if (std::holds_alternative<std::string>(roughMetTexInput))
    {
        auto texPath = get<std::string>(roughMetTexInput);
        const auto pngIdx = texPath.find("png");
        if (pngIdx != std::string::npos)
            texPath = texPath.replace(pngIdx, 3, "dds");
        roughMetTex->Init(d3d->GetDevice(), cmdList, texPath,
                         1);
    }
    else
    {
        auto pData = get<const std::byte*>(roughMetTexInput);
        roughMetTex->InitPNG(d3d->GetDevice(), cmdList, reinterpret_cast<const uint8_t*>(pData), dataSize,
                            DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    }

    std::variant<std::string, const std::byte*> emissiveTexInput = "";
    if (mat.emissiveTexture.has_value())
    {
        emissiveTexInput = loadTexture(asset, mat.emissiveTexture.value().textureIndex, dataSize);
        if (std::holds_alternative<std::string>(emissiveTexInput))
            emissiveTexInput = localDirectory + get<std::string>(emissiveTexInput);
    }
    else
        emissiveTexInput = "Assets/Textures/Black.dds";

    std::shared_ptr<Texture> emissiveTex = std::make_shared<Texture>();
    if (std::holds_alternative<std::string>(emissiveTexInput))
    {
        auto texPath = get<std::string>(emissiveTexInput);
        const auto pngIdx = texPath.find("png");
        if (pngIdx != std::string::npos)
            texPath = texPath.replace(pngIdx, 3, "dds");
        emissiveTex->Init(d3d->GetDevice(), cmdList, texPath,
                         1);
    }
    else
    {
        auto pData = get<const std::byte*>(emissiveTexInput);
        emissiveTex->InitPNG(d3d->GetDevice(), cmdList, reinterpret_cast<const uint8_t*>(pData), dataSize,
                            DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
    }

    MaterialData materialData = {};

    std::shared_ptr<Material> material = std::make_shared<Material>();
    material->Init(heap);
    material->AddCBV(d3d->GetDevice(), heap, sizeof(CbvMatrices));
    material->AddCBV(d3d->GetDevice(), heap, sizeof(CbvRasterDebug));
    material->SetTex(d3d->GetDevice(), 0, heap, diffuseTex);
    material->SetTex(d3d->GetDevice(), 1, heap, normalTex);
    material->SetTex(d3d->GetDevice(), 2, heap, roughMetTex);
    material->SetTex(d3d->GetDevice(), 3, heap, emissiveTex);

    bool isGlass = (mat.transmission && mat.transmission->transmissionFactor > 0.0) ||
        (mat.alphaMode == fastgltf::AlphaMode::Blend &&
         mat.pbrData.metallicFactor < 0.1 &&
         mat.pbrData.roughnessFactor < 0.1);

    material->SetName(mat.name.c_str());

    const bool noEmission = mat.emissiveFactor == fastgltf::math::nvec3(0,0,0) & !emissiveTex;

    memcpy(&materialData.BaseColorFactor, &mat.pbrData.baseColorFactor, sizeof(float) * 3);
    materialData.EmissiveStrength = noEmission ? 0 : mat.emissiveStrength;
    materialData.Roughness = mat.pbrData.roughnessFactor;
    materialData.Metalness = mat.pbrData.metallicFactor;
    materialData.IoR = mat.ior;
    materialData.BindlessTexDiffuse = heap->AddBindlessTexture(d3d->GetDevice(), diffuseTex);
    if (isGlass)
        materialData.Flags = PtMaterialFlags::eIsGlass;
    material->SetData(materialData);

    auto obj = std::make_shared<Object>();
    obj->Init(node.name.c_str(), t, shaderUsed, args.Root, model, material);
    args.OutObjects.emplace_back(obj);
}

void ModelLoaderGLTF::loadNode(D3D* d3d, ID3D12GraphicsCommandList* cmdList, Heap* heap, Asset& asset,
                               const std::string& modelNameExtensionless, fastgltf::Node& node, GLTFLoadArgs& args, Transform parentTransform)
{
    if (node.transform.index() != 0)
        throw std::exception("Unsupported transform type");

    auto& trs = std::get<fastgltf::TRS>(node.transform);

    const Transform localTransform = toTransform(trs);
    Transform worldTransform = {};

    XMFLOAT3 localPos = localTransform.GetPosition();
    XMVECTOR localRot = localTransform.GetRotationQ();
    const XMFLOAT3 localScale = localTransform.GetScale();

    if (args.ConvertRhToLh)
    {
        localPos.z = -localPos.z;
        localRot = XMVectorSetZ(localRot, -XMVectorGetZ(localRot)); // flip Z
        localRot = XMVectorSetY(localRot, -XMVectorGetY(localRot)); // flip Y
    }

    XMFLOAT3 worldPos = Add(parentTransform.GetPosition(), Mult(parentTransform.GetScale(), localPos));
    worldTransform.SetPosition(worldPos);

    XMVECTOR worldRot = XMQuaternionMultiply(parentTransform.GetRotationQ(), localRot);
    worldTransform.SetRotationQ(worldRot);

    const XMFLOAT3 worldScale = Mult(localScale, parentTransform.GetScale());
    worldTransform.SetScale(worldScale);

    const size_t childCount = node.children.size();
    for (size_t i = 0; i < childCount; i++)
    {
        fastgltf::Node& childNode = (*asset)->nodes[node.children[i]];
        loadNode(d3d, cmdList, heap, asset, modelNameExtensionless, childNode, args, worldTransform);
    }

    if (!node.meshIndex.has_value())
        return;

    Profiler::AddToStack(("GLTF Node: " + node.name).c_str());

    const std::string nodeName(node.name);
    if (!args.CullingWhiteList.empty())
    {
        bool found = false;
        for (const auto & j : args.CullingWhiteList)
        {
            if (nodeName.starts_with(j))
            {
                found = true;
                break;
            }
        }

        if (!found)
            return;
    }

    const size_t meshIndex = node.meshIndex.value();
    const fastgltf::Mesh& mesh = (*asset)->meshes.at(meshIndex);

    for (size_t i = 0; i < mesh.primitives.size(); i++)
    {
        const std::string id = modelNameExtensionless + "::NODE(" + std::to_string(meshIndex) + ")::PRIMITIVE(" +
            std::to_string(i) + ")";
        loadPrimitive(d3d, cmdList, heap, asset, mesh.primitives[i], modelNameExtensionless, node, args, worldTransform, id, meshIndex,
                      i);
    }

    Profiler::PopAndPrint();
}

void ModelLoaderGLTF::LoadSplitModel(D3D* d3d, ID3D12GraphicsCommandList* cmdList, Heap* heap, const std::wstring& name,
                                     GLTFLoadArgs& args, Transform transform)
{
    const std::wstring wpath = FileHelper::GetAssetModelFullPath(name.c_str());
    const std::string path = wstringToString(wpath);

    const size_t dotIndex = name.find_last_of('.');
    if (dotIndex == std::string::npos)
        throw std::exception("Invalid model name");

    const std::string modelNameExtensionless = wstringToString(name.substr(0, dotIndex));

    fastgltf::Expected<fastgltf::GltfDataBuffer> data = fastgltf::GltfDataBuffer::FromPath(path);

    if (data.error() == fastgltf::Error::InvalidPath)
    {
        CherryPrint("Error loading GLTF model (path=\"" + path + "\")");
        return;
    }

    if (data.error() != fastgltf::Error::None)
        throw std::exception("FastGLTF error");

    if (!ms_initialisedParser)
    {
        ms_parser = fastgltf::Parser(
            fastgltf::Extensions::KHR_materials_specular |
            fastgltf::Extensions::KHR_materials_iridescence |
            fastgltf::Extensions::KHR_materials_ior |
            fastgltf::Extensions::KHR_materials_emissive_strength);
        ms_initialisedParser = true;
    }

    constexpr fastgltf::Options options = fastgltf::Options::DecomposeNodeMatrices;

    Asset asset = std::make_shared<fastgltf::Expected<fastgltf::Asset>>(ms_parser.loadGltf(data.get(), path, options));
    fastgltf::Error error = asset->error();
    CherryPrint(fastgltf::getErrorName(error));

    if (asset->error() == fastgltf::Error::InvalidPath)
    {
        CherryPrint("Error loading GLTF model (path=\"" + path + "\")");
        return;
    }

    if (error != fastgltf::Error::None)
        throw std::exception("FastGLTF error");

    error = fastgltf::validate(asset->get());
    if (error != fastgltf::Error::None)
        throw std::exception("FastGLTF error");

    for (int i = 0; i < (*asset)->scenes.size(); i++)
    {
        fastgltf::Scene& scene = (*asset)->scenes[i];

        const size_t nodeCount = scene.nodeIndices.size();
        for (size_t n = 0; n < nodeCount; n++)
        {
            const size_t nodeIndex = scene.nodeIndices[n];
            fastgltf::Node& node = (*asset)->nodes[nodeIndex];
            loadNode(d3d, cmdList, heap, asset, modelNameExtensionless, node, args, transform);
        }
    }
}

void ModelLoaderGLTF::loadModelsFromNode(D3D* d3d, ID3D12GraphicsCommandList* cmdList, Asset& asset,
                                         const std::string& modelNameExtensionless, fastgltf::Node& node,
                                         std::vector<std::shared_ptr<Model>>& modelList, bool convertRhToLh)
{
    const size_t childCount = node.children.size();
    for (size_t i = 0; i < childCount; i++)
    {
        fastgltf::Node& childNode = (*asset)->nodes[node.children[i]];
        loadModelsFromNode(d3d, cmdList, asset, modelNameExtensionless, childNode, modelList, convertRhToLh);
    }

    if (!node.meshIndex.has_value())
        return;

    size_t meshIndex = node.meshIndex.value();
    fastgltf::Mesh& mesh = (*asset)->meshes.at(meshIndex);

    const uint32_t slashIdx = modelNameExtensionless.find_last_of('/');
    const std::string assetDirectory = wstringToString(FileHelper::GetAssetsPath());
    const std::string localDirectory = assetDirectory + "Models/" + modelNameExtensionless.substr(0, slashIdx) + "/";

    for (size_t i = 0; i < mesh.primitives.size(); i++)
    {
        std::string id = modelNameExtensionless + "::NODE(" + std::to_string(meshIndex) + ")::PRIMITIVE(" +
            std::to_string(i) + ")";

        std::shared_ptr<Model> model = std::make_shared<Model>();
        loadModel(d3d, cmdList, localDirectory, asset, mesh.primitives[i], model.get(), convertRhToLh);
        modelList.push_back(model);
    }
}

std::vector<std::shared_ptr<Model>> ModelLoaderGLTF::LoadModelsFromGLTF(D3D* d3d, ID3D12GraphicsCommandList* cmdList,
                                                                        const std::wstring& modelName, bool convertRhToLh)
{
    const std::wstring wpath = FileHelper::GetAssetModelFullPath(modelName.c_str());
    const std::string path = wstringToString(wpath);

    size_t dotIndex = modelName.find_last_of('.');
    if (dotIndex == std::string::npos)
        throw std::exception("Invalid model name");

    const std::string modelNameExtensionless = wstringToString(modelName.substr(0, dotIndex));

    fastgltf::Expected<fastgltf::GltfDataBuffer> data = fastgltf::GltfDataBuffer::FromPath(path);
    if (data.error() != fastgltf::Error::None)
        throw new std::exception("FastGLTF error");

    constexpr fastgltf::Options options = fastgltf::Options::None;

    Asset asset = std::make_shared<fastgltf::Expected<fastgltf::Asset>>(ms_parser.loadGltf(data.get(), path, options));
    auto error = asset->error();
    if (error != fastgltf::Error::None)
        throw new std::exception("FastGLTF error");

    error = fastgltf::validate(asset->get());
    if (error != fastgltf::Error::None)
        throw new std::exception("FastGLTF error");

    std::vector<std::shared_ptr<Model>> modelList;

    for (int i = 0; i < (*asset)->scenes.size(); i++)
    {
        fastgltf::Scene& scene = (*asset)->scenes[i];

        const size_t nodeCount = scene.nodeIndices.size();
        for (size_t n = 0; n < nodeCount; n++)
        {
            const size_t nodeIndex = scene.nodeIndices[n];
            fastgltf::Node& node = (*asset)->nodes[nodeIndex];
            loadModelsFromNode(d3d, cmdList, asset, modelNameExtensionless, node, modelList, convertRhToLh);
        }
    }

    return modelList;
}
