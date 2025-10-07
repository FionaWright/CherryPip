#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <string>

#include "fastgltf/core.hpp"
#include "fastgltf/types.hpp"
#include "Render/Transform.h"

class RootSig;
class Heap;
class Object;
class Shader;
class Model;
class D3D;

using std::ifstream;
using std::ofstream;

using Asset = std::shared_ptr<fastgltf::Expected<fastgltf::Asset>>;
using namespace DirectX;

#pragma pack(push, 1)
struct VertexInputDataGLTF
{
    XMFLOAT3 Position;
	XMFLOAT2 Texture;
	XMFLOAT3 Normal;
	XMFLOAT3 Tangent;
	XMFLOAT3 Binormal;
};
#pragma pack(pop)

struct GLTFLoadOverride
{
	UINT BatchIndex = -1;
	UINT ShaderIndex = -1;
	bool UseGlassSRVs = false;
	std::vector<std::string> WhiteList;
};

struct GLTFLoadArgs
{
	Transform Transform = {};
	std::shared_ptr<RootSig> Root;

	std::vector<std::shared_ptr<Shader>> Shaders;
	std::vector<std::shared_ptr<Object>> Objects;

	int DefaultShaderIndex = 0;
	int DefaultShaderATIndex = 1;

	std::vector<std::string> CullingWhiteList;
	std::vector<GLTFLoadOverride> Overrides;
};

class ModelLoaderGLTF
{
public:
    static std::vector<std::shared_ptr<Model>> LoadModelsFromGLTF(D3D* d3d, ID3D12GraphicsCommandList* cmdList, std::string modelName);
	static void LoadSplitModel(D3D* d3d, ID3D12GraphicsCommandList* cmdList, Heap* heap, const std::string& name, GLTFLoadArgs& args);

private:
	static Transform toTransform(fastgltf::TRS& trs);
	static void loadGLTFIndices(std::vector<uint32_t>& iBuffer, Asset& asset, const fastgltf::Primitive& primitive);
	static void loadModel(D3D* d3d, ID3D12GraphicsCommandList* cmdList, Asset& asset, const fastgltf::Primitive& primitive, Model* model);
	static std::string loadTexture(const Asset& asset, const size_t textureIndex);
	static void loadPrimitive(D3D* d3d, ID3D12GraphicsCommandList* cmdList, Heap* heap, Asset& asset, const fastgltf::Primitive& primitive, std::string modelNameExtensionless, fastgltf::Node& node, GLTFLoadArgs& args, std::string id, size_t meshIndex, size_t primitiveIndex);
	static void loadNode(D3D* d3d, ID3D12GraphicsCommandList* cmdList, Heap* heap, Asset& asset, std::string modelNameExtensionless, fastgltf::Node& node, GLTFLoadArgs& args);
	static void loadModelsFromNode(D3D* d3d, ID3D12GraphicsCommandList* cmdList, Asset& asset, std::string modelNameExtensionless, fastgltf::Node& node, std::vector<std::shared_ptr<Model>>& modelList);

	static fastgltf::Parser ms_parser;
	static bool ms_initialisedParser;
	static std::mutex ms_batchAddMutex;
};

