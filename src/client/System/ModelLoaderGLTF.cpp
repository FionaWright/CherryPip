#include "System/ModelLoaderGLTF.h"
#include <direct.h>
#include <filesystem>

#include "fastgltf/tools.hpp"
#include "HWI/Model.h"
#include "MathUtils.h"
#include "HWI/D3D.h"
#include "HWI/Texture.h"

#include "CBV.h"
#include "DualIncludes/CBV.h"

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
	XMFLOAT4 rotFloat4 = XMFLOAT4(rot.x(), rot.y(), rot.z(), rot.w());
	XMVECTOR rotVec = XMLoadFloat4(&rotFloat4);
	XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(rotVec);
	float pitch = std::atan2(rotationMatrix.r[1].m128_f32[2], rotationMatrix.r[2].m128_f32[2]);
	float yaw = std::atan2(-rotationMatrix.r[0].m128_f32[2], std::sqrt(rotationMatrix.r[1].m128_f32[2] * rotationMatrix.r[1].m128_f32[2] + rotationMatrix.r[2].m128_f32[2] * rotationMatrix.r[2].m128_f32[2]));
	float roll = std::atan2(rotationMatrix.r[0].m128_f32[1], rotationMatrix.r[0].m128_f32[0]);
	transform.SetRotation(pitch, yaw, roll);

	auto& scale = trs.scale;
	transform.SetScale(scale.x(), scale.y(), scale.z());

	return transform;
}

template<typename Func>
void loadGLTFVertexData(vector<VertexInputDataGLTF>& vBuffer, Asset& asset, const fastgltf::Primitive& primitive, const char* attribute, Func func)
{
	const fastgltf::Attribute* attributeObj = primitive.findAttribute(attribute);
	assert(attributeObj != primitive.attributes.cend());

	const auto& accessor = (*asset)->accessors.at(attributeObj->accessorIndex);
	const auto& bufferView = (*asset)->bufferViews.at(*accessor.bufferViewIndex);
	const auto& bufferData = (*asset)->buffers.at(bufferView.bufferIndex).data;

	const size_t dataOffset = bufferView.byteOffset + accessor.byteOffset;
	const size_t byteSize = fastgltf::getElementByteSize(accessor.type, accessor.componentType);
	const size_t dataStride = bufferView.byteStride.value_or(byteSize);

	const uint8_t* pData = nullptr;
	vector<uint8_t> pTempFileData;

	if (bufferData.index() == 3)
		pData = std::get<fastgltf::sources::Array>(bufferData).bytes.data() + dataOffset;
	else if (bufferData.index() == 2)
	{
		auto& uri = std::get<fastgltf::sources::URI>(bufferData);
		string path(uri.uri.path());
		std::ifstream file("Assets/Models/" + path, std::ios::binary);

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
		auto address = pData + i * dataStride;
		func(address, &vBuffer[i]);
	}
}

void ModelLoaderGLTF::loadGLTFIndices(vector<uint32_t>& iBuffer, Asset& asset, const fastgltf::Primitive& primitive)
{
	const auto& accessor = (*asset)->accessors[primitive.indicesAccessor.value()];
	const auto& bufferView = (*asset)->bufferViews[*accessor.bufferViewIndex];
	const auto& bufferData = (*asset)->buffers[bufferView.bufferIndex].data;

	const size_t dataOffset = bufferView.byteOffset + accessor.byteOffset;
	const size_t indexByteSize = fastgltf::getElementByteSize(accessor.type, accessor.componentType);
	const size_t dataStride = bufferView.byteStride.value_or(indexByteSize);

	const uint8_t* pData = nullptr;
	vector<uint8_t> pTempFileData;

	if (bufferData.index() == 3)
		pData = std::get<fastgltf::sources::Array>(bufferData).bytes.data() + dataOffset;
	else if (bufferData.index() == 2)
	{
		auto& uri = std::get<fastgltf::sources::URI>(bufferData);
		string path(uri.uri.path());
		std::ifstream file("Assets/Models/" + path, std::ios::binary);

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
		const uint8_t* const indexData0 = pData + (i + 0) * dataStride;
		const uint8_t* const indexData1 = pData + (i + 1) * dataStride;
		const uint8_t* const indexData2 = pData + (i + 2) * dataStride;

		switch (indexByteSize)
		{
		case 1:
			if (RIGHT_HANDED_TO_LEFT)
			{
				iBuffer[i + 2] = *indexData0;
				iBuffer[i + 1] = *indexData1;
				iBuffer[i + 0] = *indexData2;
				break;
			}

			iBuffer[i + 0] = *indexData0;
			iBuffer[i + 1] = *indexData1;
			iBuffer[i + 2] = *indexData2;
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

void ModelLoaderGLTF::loadModel(D3D* d3d, ID3D12GraphicsCommandList2* cmdList, Asset& asset, const fastgltf::Primitive& primitive, Model* model)
{
	vector<VertexInputDataGLTF> vertexBuffer;

	loadGLTFVertexData(vertexBuffer, asset, primitive, "POSITION", [](const uint8_t* address, VertexInputDataGLTF* output) {
		output->Position = *reinterpret_cast<const XMFLOAT3*>(address);
		if (RIGHT_HANDED_TO_LEFT)
			output->Position.x = -output->Position.x;
	});

	size_t vertexCount = vertexBuffer.size();
	if (vertexCount == 0)
		return;

	loadGLTFVertexData(vertexBuffer, asset, primitive, "TEXCOORD_0", [](const uint8_t* address, VertexInputDataGLTF* output) {
		output->Texture = *reinterpret_cast<const XMFLOAT2*>(address);
	});

	loadGLTFVertexData(vertexBuffer, asset, primitive, "NORMAL", [](const uint8_t* address, VertexInputDataGLTF* output) {
		output->Normal = Normalize(*reinterpret_cast<const XMFLOAT3*>(address));
		if (RIGHT_HANDED_TO_LEFT)
			output->Normal.x = -output->Normal.x;
	});

	loadGLTFVertexData(vertexBuffer, asset, primitive, "TANGENT", [](const uint8_t* address, VertexInputDataGLTF* output) {
		const XMFLOAT4* data = reinterpret_cast<const XMFLOAT4*>(address);
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
	XMFLOAT3 centroidFloat3 = XMFLOAT3(static_cast<float>(rollingCentroidSum.X), static_cast<float>(rollingCentroidSum.Y), static_cast<float>(rollingCentroidSum.Z));

	for (size_t j = 0; j < vertexCount; j++)
	{
		XMFLOAT3 diff = Subtract(centroidFloat3, vertexBuffer[j].Position);
		float magSq = Dot(diff, diff);
		if (magSq > boundingRadiusSq)
			boundingRadiusSq = magSq;
	}

	boundingRadiusSq = std::sqrt(boundingRadiusSq);

	vector<uint32_t> indexBuffer;
	loadGLTFIndices(indexBuffer, asset, primitive);

	model->Init(vertexBuffer.size(), indexBuffer.size(), sizeof(VertexInputDataGLTF), boundingRadiusSq, centroidFloat3);
	model->SetBuffers(cmdList, vertexBuffer.data(), indexBuffer.data());	
}

string ModelLoaderGLTF::loadTexture(Asset& asset, const size_t textureIndex)
{
	fastgltf::Texture& tex = (*asset)->textures[textureIndex];
	fastgltf::Image& image = (*asset)->images[tex.imageIndex.value()];

	if (image.data.index() == 2)
	{
		string texName(std::get<fastgltf::sources::URI>(image.data).uri.path());
		size_t slashIndex = texName.find_last_of('/');

		if (slashIndex != std::string::npos)
			texName = texName.substr(slashIndex + 1, texName.size() - slashIndex - 1);

		return texName;
	}

	if (image.data.index() == 1)
	{
		auto& bufferViewInfo = std::get<fastgltf::sources::BufferView>(image.data);
		if (bufferViewInfo.mimeType != fastgltf::MimeType::PNG)
			throw std::exception("GLB Mime type not supported");

		return std::string(image.name) + ".png";
	}

	return "";
}

void ModelLoaderGLTF::loadPrimitive(D3D* d3d, ID3D12GraphicsCommandList2* cmdList, Asset& asset, const fastgltf::Primitive& primitive, string modelNameExtensionless, fastgltf::Node& node, GLTFLoadArgs args, string id, size_t meshIndex, size_t primitiveIndex)
{
	std::shared_ptr<Model> model;
    loadModel(d3d, cmdList, asset, primitive, model.get());

	fastgltf::Material& mat = (*asset)->materials[primitive.materialIndex.value_or(0)];

	UINT shaderIndex = -1;
	UINT batchIndex = -1;
	bool useGlassSRVs = false;

	for (size_t i = 0; i < args.Overrides.size(); i++)
	{
		if (args.Overrides[i].WhiteList.size() == 0)
			continue;

		bool found = false;
		for (auto str : args.Overrides[i].WhiteList)
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
		batchIndex = args.Overrides[i].BatchIndex;
		useGlassSRVs = args.Overrides[i].UseGlassSRVs;
		break;
	}

	string diffuseTexPath = "";
	if (mat.pbrData.baseColorTexture.has_value())
		diffuseTexPath = modelNameExtensionless + "/" + loadTexture(asset, mat.pbrData.baseColorTexture.value().textureIndex);
	else if (mat.iridescence)
		diffuseTexPath = "Transparent.png";
	else
		diffuseTexPath = "WhitePOT.png";

	/*if (SettingsManager::ms_Misc.BistroLowQualityTexDiffuseEnabled && modelNameExtensionless == "Bistro")
	{
		size_t hyphenIndex = diffuseTexPath.find_last_of('-');
		string newDiffusePath = diffuseTexPath;

		if (hyphenIndex != std::string::npos)
			newDiffusePath = modelNameExtensionless + "/" + diffuseTexPath.substr(hyphenIndex + 1, diffuseTexPath.size() - hyphenIndex - 1);

		size_t _BaseColorIndex = newDiffusePath.find_last_of('_');
		if (_BaseColorIndex != std::string::npos)
			newDiffusePath.insert(_BaseColorIndex, "_0");

		if (filesystem::exists("Assets/Textures/" + newDiffusePath))
			diffuseTexPath = newDiffusePath;
	}*/

	std::shared_ptr<Texture> diffuseTex = std::make_shared<Texture>();
	diffuseTex->Init(d3d->GetDevice(), cmdList, diffuseTexPath, DXGI_FORMAT_R8G8B8A8_UNORM);

	string normalTexPath = "";
	if (mat.normalTexture.has_value())
		normalTexPath = modelNameExtensionless + "/" + loadTexture(asset, mat.normalTexture.value().textureIndex);
	else
		normalTexPath = "DefaultNormal.tga";

	/*if (SettingsManager::ms_Misc.BistroLowQualityTexNormalEnabled && modelNameExtensionless == "Bistro")
	{
		size_t hyphenIndex = normalTexPath.find_last_of('-');
		string newNormalPath = normalTexPath;

		if (hyphenIndex != std::string::npos)
			newNormalPath = modelNameExtensionless + "/" + normalTexPath.substr(hyphenIndex + 1, normalTexPath.size() - hyphenIndex - 1);

		size_t _NormalIndex = newNormalPath.find_last_of('_');
		if (_NormalIndex != std::string::npos)
			newNormalPath.insert(_NormalIndex, "_0");

		if (filesystem::exists("Assets/Textures/" + newNormalPath) && newNormalPath != "")
			normalTexPath = newNormalPath;
	}*/

	std::shared_ptr<Texture> normalTex = std::make_shared<Texture>();
	diffuseTex->Init(d3d->GetDevice(), cmdList, diffuseTexPath, DXGI_FORMAT_R8G8B8A8_UNORM);

	string specTexPath = "";
	if (mat.specular && mat.specular.get()->specularTexture.has_value())
		specTexPath = modelNameExtensionless + "/" + loadTexture(asset, mat.specular.get()->specularTexture.value().textureIndex);
	else if (mat.pbrData.metallicRoughnessTexture.has_value())
		specTexPath = modelNameExtensionless + "/" + loadTexture(asset, mat.pbrData.metallicRoughnessTexture.value().textureIndex);
	else
		specTexPath = "DefaultSpecular.png";

	//std::shared_ptr<Texture> specTex = AssetFactory::CreateTexture(specTexPath, cmdList, false, false, true);

	string thickTexPath = "";
	if (mat.iridescence && mat.iridescence->iridescenceThicknessTexture.has_value())
		thickTexPath = modelNameExtensionless + "/" + loadTexture(asset, mat.iridescence->iridescenceThicknessTexture.value().textureIndex);
	else
		thickTexPath = "WhitePOT.png";

	//std::shared_ptr<Texture> thickTex = AssetFactory::CreateTexture(thickTexPath, cmdList);

	string iridTexPath = "";
	if (mat.iridescence && mat.iridescence->iridescenceTexture.has_value())
		iridTexPath = modelNameExtensionless + "/" + loadTexture(asset, mat.iridescence->iridescenceTexture.value().textureIndex);
	else
		iridTexPath = "WhitePOT.png";

	//std::shared_ptr<Texture> iridTex = AssetFactory::CreateTexture(iridTexPath, cmdList);

	//std::shared_ptr<Texture> blueNoiseTex = AssetFactory::CreateTexture("BlueNoise.png", cmdList);
	//std::shared_ptr<Texture> brdfIntTex = AssetFactory::CreateTexture("BRDF Integration Map.png", cmdList);

	vector<UINT> cbvSizesDraw = { sizeof(CbvMatrices) };
	vector<std::shared_ptr<Texture>> textures = { diffuseTex };

	std::unique_lock<std::mutex> lock(ms_batchAddMutex);
	std::shared_ptr<Material> material = AssetFactory::CreateMaterial();
	material->AddCBVs(d3d, cmdList, cbvSizesDraw, false);
	material->AddCBVs(d3d, cmdList, cbvSizesFrame, true);
	material->AddSRVs(d3d, textures);
	material->AddDynamicSRVs("Shadow Map", 1);
	lock.unlock();

	material->SetCBV_PerDraw(1, &matProperties, sizeof(MaterialPropertiesCB));
	material->SetCBV_PerDraw(2, &thinFilm, sizeof(ThinFilmCB));
	material->AttachProperties(matProperties);
	material->AttachThinFilm(thinFilm);	

	string nodeName(node.name);
	nodeName = id + "::" + nodeName;

	bool alphaRequirementMet = SettingsManager::ms_Misc.RequireAlphaTextureForDoubleSided ? material->GetHasAlpha() : true;
	bool isAT = alphaRequirementMet || mat.doubleSided;

	if (shaderIndex == -1)
		shaderIndex = isAT ? args.DefaultShaderATIndex : args.DefaultShaderIndex;
	if (batchIndex == -1)
		batchIndex = args.DefaultBatchIndex;

	auto& shaderUsed = args.Shaders[shaderIndex];
	GameObject go(nodeName, model, shaderUsed, material);
	go.SetTransform(args.Transform);

	if (useGlassSRVs)
		go.ForceSetTransparent(true);
	else
		go.ForceSetAT(isAT);

	std::unique_lock<std::mutex> lock2(ms_batchAddMutex);
	args.Batches[batchIndex]->AddGameObject(go);
}

void ModelLoaderGLTF::LoadNode(D3D* d3d, ID3D12GraphicsCommandList2* cmdList, Asset& asset, string modelNameExtensionless, fastgltf::Node& node, GLTFLoadArgs args)
{
	Transform localTransform;
	if (node.transform.index() == 0)
	{
		fastgltf::TRS& trs = std::get<fastgltf::TRS>(node.transform);
		localTransform = ToTransform(trs);
	}
	else
	{
		throw std::exception("Unsupported transform type");
	}

	Transform worldTransform = localTransform;
	worldTransform.Position.x = -localTransform.Position.x;

	worldTransform.Position = Add(worldTransform.Position, args.Transform.Position);
	worldTransform.Rotation = Add(worldTransform.Rotation, args.Transform.Rotation);
	worldTransform.Scale = Mult(worldTransform.Scale, args.Transform.Scale);

	size_t childCount = node.children.size();
	for (size_t i = 0; i < childCount; i++)
	{
		fastgltf::Node& childNode = (*asset)->nodes[node.children[i]];
		args.Transform = worldTransform;
		LoadNode(d3d, cmdList, asset, modelNameExtensionless, childNode, args);
	}

	if (!node.meshIndex.has_value())
		return;

	worldTransform.Position = Mult(worldTransform.Position, worldTransform.Scale);
	args.Transform = worldTransform;

	string nodeName(node.name);
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
		std::string id = modelNameExtensionless + "::NODE(" + std::to_string(meshIndex) + ")::PRIMITIVE(" + std::to_string(i) + ")";
		if (SettingsManager::ms_DX12.Async.AsyncGLTFLoadingEnabled)
			threads.emplace_back(std::jthread(LoadPrimitive, d3d, cmdList, std::ref(asset), std::ref(mesh.primitives[i]), modelNameExtensionless, std::ref(node), args, id, meshIndex, i));
		else
			LoadPrimitive(d3d, cmdList, asset, mesh.primitives[i], modelNameExtensionless, node, args, id, meshIndex, i);
	}
}

void ModelLoaderGLTF::LoadSplitModel(D3D* d3d, ID3D12GraphicsCommandList2* cmdList, string modelName, GLTFLoadArgs& args)
{
	string path = "Assets/Models/" + modelName;

	size_t dotIndex = modelName.find_last_of('.');
	if (dotIndex == string::npos)
		throw std::exception("Invalid model name");

	string modelNameExtensionless = modelName.substr(0, dotIndex);

	fastgltf::Expected<fastgltf::GltfDataBuffer> data = fastgltf::GltfDataBuffer::FromPath(path);

	if (data.error() == fastgltf::Error::InvalidPath)
	{
		AlkaliGUIManager::LogErrorMessage("Error loading GLTF model (path=\"" + path + "\")");
		return;
	}

	if (data.error() != fastgltf::Error::None)
		throw new std::exception("FastGLTF error");

	if (!ms_initialisedParser)
	{
		ms_parser = fastgltf::Parser(fastgltf::Extensions::KHR_materials_specular | fastgltf::Extensions::KHR_materials_iridescence);
		ms_initialisedParser = true;
	}

	fastgltf::Options options = fastgltf::Options::DecomposeNodeMatrices;

	Asset asset = std::make_shared<fastgltf::Expected<fastgltf::Asset>>(ms_parser.loadGltf(data.get(), path, options));

	if (asset->error() == fastgltf::Error::InvalidPath)
	{
		AlkaliGUIManager::LogErrorMessage("Error loading GLTF model (path=\"" + path + "\")");
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

		size_t nodeCount = scene.nodeIndices.size();
		for (size_t n = 0; n < nodeCount; n++)
		{
			size_t nodeIndex = scene.nodeIndices[n];
			fastgltf::Node& node = (*asset)->nodes[nodeIndex];
			LoadNode(d3d, cmdList, asset, modelNameExtensionless, node, args);
		}
	}
}

void ModelLoaderGLTF::LoadModelsFromNode(D3D* d3d, ID3D12GraphicsCommandList2* cmdList, Asset& asset, string modelNameExtensionless, fastgltf::Node& node, vector<std::shared_ptr<Model>>& modelList)
{
	size_t childCount = node.children.size();
	for (size_t i = 0; i < childCount; i++)
	{
		fastgltf::Node& childNode = (*asset)->nodes[node.children[i]];
		LoadModelsFromNode(d3d, cmdList, asset, modelNameExtensionless, childNode, modelList);
	}

	if (!node.meshIndex.has_value())
		return;

	size_t meshIndex = node.meshIndex.value();
	fastgltf::Mesh& mesh = (*asset)->meshes.at(meshIndex);

	for (size_t i = 0; i < mesh.primitives.size(); i++)
	{
		std::string id = modelNameExtensionless + "::NODE(" + std::to_string(meshIndex) + ")::PRIMITIVE(" + std::to_string(i) + ")";

		std::shared_ptr<Model> model;
		if (!ResourceTracker::TryGetModel(id, model))
		{
			bool successfulAsyncPush = SettingsManager::ms_DX12.Async.Enabled && SettingsManager::ms_DX12.Async.LoadModels && LoadManager::TryPushModel(model.get(), asset, meshIndex, i);
			if (!successfulAsyncPush)
			{
				LoadModel(d3d, cmdList, asset, mesh.primitives[i], model.get());
				model->MarkLoaded();
			}
		}
		modelList.push_back(model);
	}
}

vector<std::shared_ptr<Model>> ModelLoaderGLTF::LoadModelsFromGLTF(D3D* d3d, ID3D12GraphicsCommandList2* cmdList, string modelName)
{
	string path = "Assets/Models/" + modelName;

	size_t dotIndex = modelName.find_last_of('.');
	if (dotIndex == string::npos)
		throw std::exception("Invalid model name");

	string modelNameExtensionless = modelName.substr(0, dotIndex);

	fastgltf::Expected<fastgltf::GltfDataBuffer> data = fastgltf::GltfDataBuffer::FromPath(path);
	if (data.error() != fastgltf::Error::None)
		throw new std::exception("FastGLTF error");

	fastgltf::Options options = fastgltf::Options::None;

	Asset asset = std::make_shared<fastgltf::Expected<fastgltf::Asset>>(ms_parser.loadGltf(data.get(), path, options));
	auto error = asset->error();
	if (error != fastgltf::Error::None)
		throw new std::exception("FastGLTF error");

	error = fastgltf::validate(asset->get());
	if (error != fastgltf::Error::None)
		throw new std::exception("FastGLTF error");

	vector<std::shared_ptr<Model>> modelList;

	for (int i = 0; i < (*asset)->scenes.size(); i++)
	{
		fastgltf::Scene& scene = (*asset)->scenes[i];

		size_t nodeCount = scene.nodeIndices.size();
		for (size_t n = 0; n < nodeCount; n++)
		{
			size_t nodeIndex = scene.nodeIndices[n];
			fastgltf::Node& node = (*asset)->nodes[nodeIndex];
			LoadModelsFromNode(d3d, cmdList, asset, modelNameExtensionless, node, modelList);
		}
	}

	return modelList;
}