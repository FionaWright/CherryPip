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
#include "HWI/BLAS.h"
#include "HWI/Material.h"
#include "Render/Object.h"
#include "System/FileHelper.h"
#include "System/TextureLoader.h"

namespace filesystem = std::filesystem;

fastgltf::Parser ModelLoaderGLTF::ms_parser;
bool ModelLoaderGLTF::ms_initialisedParser;
std::mutex ModelLoaderGLTF::ms_batchAddMutex;

constexpr bool RIGHT_HANDED_TO_LEFT = true;

Transform ModelLoaderGLTF::toTransform(fastgltf::TRS& trs)
{
    Transform transform;

    auto& pos = trs.translation;
    transform.SetPosition(pos.x(), pos.y(), pos.z());

    auto& rot = trs.rotation;
    const XMFLOAT4 rotFloat4 = XMFLOAT4(rot.x(), rot.y(), rot.z(), rot.w());
    const XMVECTOR rotVec = XMLoadFloat4(&rotFloat4);
    const XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(rotVec);
    float pitch = std::atan2(rotationMatrix.r[1].m128_f32[2], rotationMatrix.r[2].m128_f32[2]);
    float yaw = std::atan2(-rotationMatrix.r[0].m128_f32[2],
                           std::sqrt(
                               rotationMatrix.r[1].m128_f32[2] * rotationMatrix.r[1].m128_f32[2] + rotationMatrix.r[2].
                               m128_f32[2] * rotationMatrix.r[2].m128_f32[2]));
    float roll = std::atan2(rotationMatrix.r[0].m128_f32[1], rotationMatrix.r[0].m128_f32[0]);
    transform.SetRotation(pitch, yaw, roll);

    auto& scale = trs.scale;
    transform.SetScale(scale.x(), scale.y(), scale.z());

    return transform;
}

template <typename Func>
void loadGLTFVertexData(const std::string& directory, std::vector<VertexInputDataGLTF>& vBuffer, Asset& asset,
                        const fastgltf::Primitive& primitive, const char* attribute, Func func)
{
    const fastgltf::Attribute* attributeObj = primitive.findAttribute(attribute);
    assert(attributeObj != primitive.attributes.cend());

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
                                      const fastgltf::Primitive& primitive)
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
        std::string path(uri.uri.path());
        std::ifstream file(directory + path, std::ios::binary);

        if (!file.is_open())
            throw std::runtime_error("Failed to open file");

        file.seekg(dataOffset + uri.fileByteOffset, std::ios::beg);

        size_t totalBytes = accessor.count * indexByteSize;
        pTempFileData.resize(totalBytes);
        file.read(reinterpret_cast<char*>(pTempFileData.data()), totalBytes);
        file.close();

        pData = pTempFileData.data();

        if (!file)
            throw std::runtime_error("Failed to read the required data from file.");
    }
    else
        throw new std::exception("Invalid buffer data type");

    iBuffer.resize(accessor.count);

    if (!RIGHT_HANDED_TO_LEFT && indexByteSize == 4)
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
            if (RIGHT_HANDED_TO_LEFT)
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

            if (RIGHT_HANDED_TO_LEFT)
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

void ModelLoaderGLTF::loadModel(D3D* d3d, ID3D12GraphicsCommandList* cmdList, const std::string& directory,
                                Asset& asset, const fastgltf::Primitive& primitive, Model* model)
{
    std::vector<VertexInputDataGLTF> vertexBuffer;

    loadGLTFVertexData(directory, vertexBuffer, asset, primitive, "POSITION",
                       [](const std::byte* address, VertexInputDataGLTF* output)
                       {
                           output->Position = *reinterpret_cast<const XMFLOAT3*>(address);
                           if (RIGHT_HANDED_TO_LEFT)
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
                       [](const std::byte* address, VertexInputDataGLTF* output)
                       {
                           output->Normal = Normalize(*reinterpret_cast<const XMFLOAT3*>(address));
                           if (RIGHT_HANDED_TO_LEFT)
                               output->Normal.x = -output->Normal.x;
                       });

    loadGLTFVertexData(directory, vertexBuffer, asset, primitive, "TANGENT",
                       [](const std::byte* address, VertexInputDataGLTF* output)
                       {
                           const auto* data = reinterpret_cast<const XMFLOAT4*>(address);
                           float handedness = data->w > 0.0f ? 1.0f : -1.0f;
                           output->Tangent = Normalize(Mult(XMFLOAT3(data->x, data->y, data->z), handedness));
                           if (RIGHT_HANDED_TO_LEFT)
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
    XMFLOAT3 centroidFloat3 = XMFLOAT3(static_cast<float>(rollingCentroidSum.X),
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
    loadGLTFIndices(directory, indexBuffer, asset, primitive);

    model->Init(d3d->GetDevice(), vertexBuffer.size(), indexBuffer.size(), sizeof(VertexInputDataGLTF),
                boundingRadiusSq, centroidFloat3);
    model->SetBuffers(d3d->GetDevice(), cmdList, vertexBuffer.data(), indexBuffer.data());
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
                                    fastgltf::Node& node, GLTFLoadArgs& args, std::string id, size_t meshIndex,
                                    size_t primitiveIndex)
{
    const uint32_t slashIdx = modelNameExtensionless.find_last_of('/');
    const std::string directory = "Assets/Models/" + modelNameExtensionless.substr(0, slashIdx) + "/";

    std::shared_ptr<Model> model = std::make_shared<Model>();
    loadModel(d3d, cmdList, directory, asset, primitive, model.get());

    fastgltf::Material& mat = (*asset)->materials[primitive.materialIndex.value_or(0)];

    UINT shaderIndex = -1;

    for (size_t i = 0; i < args.Overrides.size(); i++)
    {
        if (args.Overrides[i].WhiteList.empty())
            continue;

        bool found = false;
        for (const auto& str : args.Overrides[i].WhiteList)
        {
            if (node.name.starts_with(str))
            {
                found = true;
                break;
            }
        }
        if (!found)
            continue;

        shaderIndex = args.Overrides[i].ShaderIndex;
        break;
    }

    std::string nodeName(node.name);
    nodeName = id + "::" + nodeName;

    auto transform = std::make_shared<Transform>();
    *transform = args.Transform;

    if (args.ExportBlasModeEnabled)
    {
        ComPtr<ID3D12Device5> device5;
        V(d3d->GetDevice()->QueryInterface(IID_PPV_ARGS(&device5)));

        ComPtr<ID3D12GraphicsCommandList4> cmdList4;
        V(cmdList->QueryInterface(IID_PPV_ARGS(&cmdList4)));

        auto blas = std::make_shared<BLAS>();
        blas->Init(device5.Get(), cmdList4.Get(), model, *transform.get());
        args.BLASs.emplace_back(blas);
    }
    else
    {
        std::variant<std::string, const std::byte*> diffuseTexInput = "";
        size_t dataSize = 0;
        if (mat.pbrData.baseColorTexture.has_value())
        {
            diffuseTexInput = loadTexture(asset, mat.pbrData.baseColorTexture.value().textureIndex, dataSize);
            if (std::holds_alternative<std::string>(diffuseTexInput))
                diffuseTexInput = directory + get<std::string>(diffuseTexInput);
        }
        else if (mat.iridescence)
            diffuseTexInput = "Transparent.png";
        else
            diffuseTexInput = "Assets/Textures/WhitePOT.png";

        std::shared_ptr<Texture> diffuseTex = std::make_shared<Texture>();
        if (std::holds_alternative<std::string>(diffuseTexInput))
        {
            diffuseTex->Init(d3d->GetDevice(), cmdList, get<std::string>(diffuseTexInput), DXGI_FORMAT_R8G8B8A8_UNORM,
                             1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
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
                normalTexInput = directory + get<std::string>(normalTexInput);
        }
        else
            normalTexInput = "Assets/Textures/DefaultNormal.tga";

        std::shared_ptr<Texture> normalTex = std::make_shared<Texture>();
        if (std::holds_alternative<std::string>(normalTexInput))
        {
            //normalTex->Init(d3d->GetDevice(), cmdList, get<std::string>(normalTexInput), DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        }
        else
        {
            //normalTex->InitPNG(d3d->GetDevice(), cmdList, get<const std::byte*>(normalTexInput), DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
        }

        std::shared_ptr<Material> material = std::make_shared<Material>();
        material->Init(heap);
        material->AddCBV(d3d->GetDevice(), heap, sizeof(CbvMatrices));
        material->AddSRV(d3d->GetDevice(), heap, diffuseTex);

        if (shaderIndex == -1)
            shaderIndex = args.DefaultShaderIndex;

        auto& shaderUsed = args.Shaders[shaderIndex];

        auto obj = std::make_shared<Object>();
        obj->Init(transform, shaderUsed, args.Root, model, material);
        args.Objects.emplace_back(obj);
    }
}

void ModelLoaderGLTF::loadNode(D3D* d3d, ID3D12GraphicsCommandList* cmdList, Heap* heap, Asset& asset,
                               const std::string& modelNameExtensionless, fastgltf::Node& node, GLTFLoadArgs& args)
{
    if (node.transform.index() != 0)
        throw std::exception("Unsupported transform type");

    fastgltf::TRS& trs = std::get<fastgltf::TRS>(node.transform);

    Transform localTransform = toTransform(trs);
    Transform worldTransform = {};

    XMFLOAT3 pos = localTransform.GetPosition();
    pos.x = -pos.x;
    pos = Add(pos, args.Transform.GetPosition());

    worldTransform.SetPosition(pos);

    XMFLOAT3 rot = localTransform.GetRotation();
    rot = Add(rot, args.Transform.GetRotation());

    worldTransform.SetRotation(rot);

    XMFLOAT3 scale = localTransform.GetScale();
    scale = Mult(scale, args.Transform.GetScale());

    worldTransform.SetScale(scale);

    size_t childCount = node.children.size();
    for (size_t i = 0; i < childCount; i++)
    {
        fastgltf::Node& childNode = (*asset)->nodes[node.children[i]];
        args.Transform = worldTransform;
        loadNode(d3d, cmdList, heap, asset, modelNameExtensionless, childNode, args);
    }

    if (!node.meshIndex.has_value())
        return;

    //pos = Mult(pos, scale);
    //worldTransform.SetPosition(pos);

    args.Transform = worldTransform;

    std::string nodeName(node.name);
    if (args.CullingWhiteList.size() > 0)
    {
        bool found = false;
        for (int j = 0; j < args.CullingWhiteList.size(); j++)
        {
            if (nodeName.starts_with(args.CullingWhiteList[j]))
            {
                found = true;
                break;
            }
        }

        if (!found)
            return;
    }

    size_t meshIndex = node.meshIndex.value();
    fastgltf::Mesh& mesh = (*asset)->meshes.at(meshIndex);

    std::vector<std::jthread> threads;

    for (size_t i = 0; i < mesh.primitives.size(); i++)
    {
        std::string id = modelNameExtensionless + "::NODE(" + std::to_string(meshIndex) + ")::PRIMITIVE(" +
            std::to_string(i) + ")";
        loadPrimitive(d3d, cmdList, heap, asset, mesh.primitives[i], modelNameExtensionless, node, args, id, meshIndex,
                      i);
    }
}

void ModelLoaderGLTF::LoadSplitModel(D3D* d3d, ID3D12GraphicsCommandList* cmdList, Heap* heap, const std::wstring& name,
                                     GLTFLoadArgs& args)
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
        std::cout << "Error loading GLTF model (path=\"" + path + "\")" << std::endl;
        return;
    }

    if (data.error() != fastgltf::Error::None)
        throw new std::exception("FastGLTF error");

    if (!ms_initialisedParser)
    {
        ms_parser = fastgltf::Parser(
            fastgltf::Extensions::KHR_materials_specular | fastgltf::Extensions::KHR_materials_iridescence);
        ms_initialisedParser = true;
    }

    constexpr fastgltf::Options options = fastgltf::Options::DecomposeNodeMatrices;

    Asset asset = std::make_shared<fastgltf::Expected<fastgltf::Asset>>(ms_parser.loadGltf(data.get(), path, options));

    if (asset->error() == fastgltf::Error::InvalidPath)
    {
        std::cout << "Error loading GLTF model (path=\"" + path + "\")" << std::endl;
        return;
    }

    if (asset->error() != fastgltf::Error::None)
        throw new std::exception("FastGLTF error");

    auto error = fastgltf::validate(asset->get());
    if (error != fastgltf::Error::None)
        throw new std::exception("FastGLTF error");

    for (int i = 0; i < (*asset)->scenes.size(); i++)
    {
        fastgltf::Scene& scene = (*asset)->scenes[i];

        const size_t nodeCount = scene.nodeIndices.size();
        for (size_t n = 0; n < nodeCount; n++)
        {
            const size_t nodeIndex = scene.nodeIndices[n];
            fastgltf::Node& node = (*asset)->nodes[nodeIndex];
            loadNode(d3d, cmdList, heap, asset, modelNameExtensionless, node, args);
        }
    }
}

void ModelLoaderGLTF::loadModelsFromNode(D3D* d3d, ID3D12GraphicsCommandList* cmdList, Asset& asset,
                                         const std::string& modelNameExtensionless, fastgltf::Node& node,
                                         std::vector<std::shared_ptr<Model>>& modelList)
{
    const size_t childCount = node.children.size();
    for (size_t i = 0; i < childCount; i++)
    {
        fastgltf::Node& childNode = (*asset)->nodes[node.children[i]];
        loadModelsFromNode(d3d, cmdList, asset, modelNameExtensionless, childNode, modelList);
    }

    if (!node.meshIndex.has_value())
        return;

    size_t meshIndex = node.meshIndex.value();
    fastgltf::Mesh& mesh = (*asset)->meshes.at(meshIndex);

    for (size_t i = 0; i < mesh.primitives.size(); i++)
    {
        std::string id = modelNameExtensionless + "::NODE(" + std::to_string(meshIndex) + ")::PRIMITIVE(" +
            std::to_string(i) + ")";

        std::shared_ptr<Model> model;
        /*if (!ResourceTracker::TryGetModel(id, model))
        {
            loadModel(d3d, cmdList, asset, mesh.primitives[i], model.get());
        }*/
        loadModel(d3d, cmdList, "Assets/Models/", asset, mesh.primitives[i], model.get());
        modelList.push_back(model);
    }
}

std::vector<std::shared_ptr<Model>> ModelLoaderGLTF::LoadModelsFromGLTF(D3D* d3d, ID3D12GraphicsCommandList* cmdList,
                                                                        std::string modelName)
{
    std::string path = "Assets/Models/" + modelName;

    size_t dotIndex = modelName.find_last_of('.');
    if (dotIndex == std::string::npos)
        throw std::exception("Invalid model name");

    std::string modelNameExtensionless = modelName.substr(0, dotIndex);

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
            loadModelsFromNode(d3d, cmdList, asset, modelNameExtensionless, node, modelList);
        }
    }

    return modelList;
}
