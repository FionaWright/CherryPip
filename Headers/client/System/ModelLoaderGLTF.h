#pragma once

#include "Render/Transform.h"

#include "fastgltf/core.hpp"
#include "fastgltf/types.hpp"
class D12Resource;
class Texture;
using Asset = std::shared_ptr<fastgltf::Expected<fastgltf::Asset>>;

class RootSig;
class Heap;
class Object;
class BLAS;
class Shader;
class Model;
class D3D;

using std::ifstream;
using std::ofstream;

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
	std::shared_ptr<RootSig> Root;
	std::vector<std::shared_ptr<Shader>> Shaders;
	D12Resource* IrradianceMap;

	int DefaultShaderIndex = -1;
	int DefaultShaderATIndex = -1;
	bool ConvertRhToLh = true;

	std::vector<std::string> CullingWhiteList;
	std::vector<GLTFLoadOverride> Overrides;

    std::vector<std::shared_ptr<Object>> OutObjects;
};

class ModelLoaderGLTF
{
public:
    static std::vector<std::shared_ptr<Model>> LoadModelsFromGLTF(D3D* d3d, ID3D12GraphicsCommandList* cmdList, const std::wstring& modelName, bool convertRhToLh);
	static void LoadSplitModel(D3D* d3d, ID3D12GraphicsCommandList* cmdList, Heap* heap, const std::wstring& name, GLTFLoadArgs& args, Transform transform);

private:
	static Transform toTransform(fastgltf::TRS& trs);
	static void loadGLTFIndices(const std::string& directory, std::vector<uint32_t>& iBuffer, Asset& asset, const fastgltf::Primitive& primitive, bool convertRhToLh);
	static void loadModel(const D3D* d3d, ID3D12GraphicsCommandList* cmdList, const std::string& directory, Asset& asset, const fastgltf::Primitive& primitive, Model* model, bool convertRhToLh);

	template<typename T>
	static std::shared_ptr<Texture> loadTextureResource(const D3D* d3d, ID3D12GraphicsCommandList* cmdList, const Asset& asset, const fastgltf::Optional<T>& gltfTex, const std::string& localDir, const char* backupPath);
	static std::variant<std::string, const std::byte*> loadTexture(const Asset& asset, size_t textureIndex, size_t& outDataSize);
	static void loadPrimitive(D3D* d3d, ID3D12GraphicsCommandList* cmdList, Heap* heap, Asset& asset, const fastgltf::Primitive& primitive, const std::string& modelNameExtensionless, fastgltf::Node& node, GLTFLoadArgs& args, Transform transform, std::string id, size_t meshIndex, size_t primitiveIndex);
	static void loadNode(D3D* d3d, ID3D12GraphicsCommandList* cmdList, Heap* heap, Asset& asset, const std::string& modelNameExtensionless, fastgltf::Node& node, GLTFLoadArgs& args, Transform parentTransform);
	static void loadModelsFromNode(D3D* d3d, ID3D12GraphicsCommandList* cmdList, Asset& asset, const std::string& modelNameExtensionless, fastgltf::Node& node, std::vector<std::shared_ptr<Model>>& modelList, bool convertRhToLh);

	static fastgltf::Parser ms_parser;
	static bool ms_initialisedParser;
	static std::mutex ms_batchAddMutex;
};

