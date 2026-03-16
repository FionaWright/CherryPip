#include "System/pch.h"

#include "Apps/SceneStudio/Headers/PathTracer.h"

#include "Helper.h"
#include "Apps/SceneStudio/Headers/SceneStudio.h"
#include "HWI/D3D.h"
#include "Render/TextureRTV.h"

void PathTracer::Init(D3D* d3d, ID3D12GraphicsCommandList* cmdList, Heap* heap, TextureRTV* rtvForReadback)
{
    Config::SetUIntFromArg(&m_config.SPP, "--spp");
    Config::SetBoolFromArg(&m_config.DirLightEnabled, "--dirLight");
    Config::SetBoolFromArg(&m_config.RussianRouletteEnabled, "--russianRoulette");
    Config::SetUIntFromArg(reinterpret_cast<uint32_t*>(&m_config.LightingModel), "--lightingModel");

    m_shader = std::make_shared<Shader>();

    ID3D12Device* device = d3d->GetDevice();

    D3D12_STATIC_SAMPLER_DESC samplers[1];
    samplers[0] = {};
    samplers[0].Filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
    samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    samplers[0].ShaderRegister = 0;

    m_rootSig = std::make_shared<RootSig>();
    m_rootSig->SmartInit(device, 3, 6, 2, true, samplers, _countof(samplers));

    m_ptContext.Init(device, cmdList, heap);

#ifdef _DEBUG
    m_pathVisualizer.Init(d3d, cmdList, 200);

    m_readbackManager.Init(d3d, heap, rtvForReadback);
#endif
}

void PathTracer::BuildScene(D3D* d3d, ID3D12GraphicsCommandList* cmdList, EnvMap* envMap, Scene* scene, Heap* heap)
{
    if (d3d->GetRayTracingSupported())
    {
        D12Resource* envMapResource = m_config.EnvMapIsEqualArea ? envMap->GetEA() : envMap->GetPano();
        m_ptContext.BuildScene(d3d->GetDevice(), cmdList, scene, heap, envMapResource);
    }
}

void PathTracer::Update(D3D* d3d, Heap* heap, bool shaderDirty, bool isSpectral, bool envMapEnabled)
{
    if (shaderDirty)
        compilePtShader(d3d, isSpectral, envMapEnabled);

    if (m_config.DebugPathVisualization)
        m_ptContext.SetMaterialPathVisualizationBuffer(d3d->GetDevice(), heap, m_pathVisualizer.GetStructuredBuffer(),
                                                       m_pathVisualizer.GetNumElements());
}

void PathTracer::PostUpdate(D3D* d3d, Heap* heap,  const std::shared_ptr<Shader>& shaderLine)
{
#ifdef _DEBUG
    if (m_completedPathVisualizationSnapshot)
    {
        const auto data = m_pathVisualizer.ReadbackData(d3d);

        m_pathVisualizationLines.clear();
        for (int i = 0; i < m_config.SPP; i++)
        {
            for (int j = 0; j < data[i].NumPositionsSet - 1; j++)
            {
                const XMFLOAT3 start = data[i].WorldSpacePositionAtBounce[j];
                const XMFLOAT3 end = data[i].WorldSpacePositionAtBounce[j + 1];

                const float bounceT = j / static_cast<float>(m_config.NumBounces);
                const XMFLOAT3 color = XMFLOAT3(1.0f - bounceT, bounceT, 0);

                auto line = std::make_shared<DebugLine>();
                line->Init(d3d, heap, shaderLine);
                line->Update(d3d, &start, &end, &color);
                m_pathVisualizationLines.emplace_back(line);
            }
        }

        m_completedPathVisualizationSnapshot = false;
    }
#endif
}

void PathTracer::Render(D3D* d3d, ID3D12GraphicsCommandList* cmdList, Heap* heap, const Camera* camera,
                        const XMMATRIX& projMatrix, CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle, TextureRTV* rtvTex,
                        XMFLOAT2 mousePosOnClick, const DirLightConfig& dirLightConfig)
{
    if (!d3d->GetRayTracingSupported())
    {
        std::cout << "ERROR: Ray-Tracing not supported!!!" << std::endl;
        return;
    }

    const float fRtvWidth = static_cast<float>(Config::GetSystem().RtvWidth);
    const float fRtvHeight = static_cast<float>(Config::GetSystem().RtvHeight);

    const CD3DX12_VIEWPORT viewport(0.0f, 0.0f, fRtvWidth, fRtvHeight);
    const CD3DX12_RECT scissorRect(0, 0, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight);

    cmdList->RSSetViewports(1, &viewport);
    cmdList->RSSetScissorRects(1, &scissorRect);

    // Main Pass (Into PP1)
    {
        cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

        const int debugBufferIdx = m_config.DebugInfoOutputEnabled
                                       ? static_cast<int>(m_config.DebugInfoOutputMode)
                                       : -1;

        m_ptContext.Render(cmdList, m_rootSig->Get(), m_shader->GetPSO(),
                           camera, heap, projMatrix,
                           m_config,
                           dirLightConfig.DirLightIntensity, dirLightConfig.DirLightColor,
                           dirLightConfig.DirLightDirection
#ifdef _DEBUG
                           , debugBufferIdx, m_takePathVisualizationSnapshot, mousePosOnClick
#endif
        );
    }

#ifdef _DEBUG
    if (m_takePathVisualizationSnapshot)
    {
        m_completedPathVisualizationSnapshot = true;
        m_takePathVisualizationSnapshot = false;
    }

    // Readback Pass (PP1 to RTV)
    if (m_config.ReadbackEnabled)
    {
        m_readbackManager.ReadbackPass(d3d, cmdList, rtvTex, m_config.ReadbackEveryFrame, mousePosOnClick);
        // TODO: Is this needed? 
        //copyRtvTex(cmdList, d3d->GetRtv(), m_rtvPingPong1.GetD12Resource());
        //return;
    }
#endif
}

void PathTracer::RenderLines(D3D* d3d, ID3D12GraphicsCommandList* cmdList, const XMMATRIX& vpMatrix)
{
    for (int i = 0; i < m_pathVisualizationLines.size(); i++)
        m_pathVisualizationLines[i]->Render(cmdList, vpMatrix);
}

void PathTracer::Reset()
{
    m_ptContext.Reset();
#ifdef _DEBUG
    m_readbackManager.ClearReadbackData();
#endif
}

void PathTracer::compilePtShader(const D3D* d3d, bool isSpectral, bool envMapEnabled)
{
    if (!d3d->GetRayTracingSupported())
        return;

    CherryPrint("Loading Path-Tracing Shader");

    std::vector<std::wstring> args = {};
    if (m_config.DebugInfoOutputEnabled)
        args.push_back(L"-DDEBUG_PT_INFO_OUTPUT");
    if (m_config.DebugForceSpecular)
        args.push_back(L"-DDEBUG_FORCE_SPECULAR");
    if (m_config.DebugForceDiffuse)
        args.push_back(L"-DDEBUG_FORCE_DIFFUSE");
    if (m_config.DebugForceReflect)
        args.push_back(L"-DDEBUG_FORCE_REFLECT");
    if (m_config.DebugForceRefract)
        args.push_back(L"-DDEBUG_FORCE_REFRACT");

    if (envMapEnabled)
        args.push_back(L"-DENV_MAP_ENABLED");
    if (m_config.EnvMapIsEqualArea)
        args.push_back(L"-DENV_MAP_EA");
    if (m_config.DirLightEnabled)
        args.push_back(L"-DDIR_LIGHT_ENABLED");
    if (m_config.NormalMapsEnabled)
        args.push_back(L"-DNORMAL_MAPS_ENABLED");
    if (m_config.AlphaTestingEnabled)
        args.push_back(L"-DALPHA_TESTING_ENABLED");
    if (m_config.RussianRouletteEnabled)
        args.push_back(L"-DRUSSIAN_ROULETTE_ENABLED");
    if (m_config.SampleVisibleNormals)
        args.push_back(L"-DSAMPLE_VISIBLE_NORMALS");
    if (m_config.AnisotropyEnabled)
        args.push_back(L"-DANISOTROPY_ENABLED");
    if (m_config.DepthOfFieldEnabled)
        args.push_back(L"-DDEPTH_OF_FIELD_ENABLED");
    if (m_config.ImportanceSamplingEnabled)
        args.push_back(L"-DIMPORTANCE_SAMPLING");
    if (m_config.GlassModelEnabled)
        args.push_back(L"-DLIGHTING_GLASS_ENABLED");
    if (m_config.JitterEnabled)
        args.push_back(L"-DJITTER_ENABLED");
    if (m_config.DirLightIsDistant)
        args.push_back(L"-DDIR_LIGHT_DISTANT");
    if (m_config.GammaCorrection)
        args.push_back(L"-DGAMMA_CORRECTION");

    if (m_config.FurnaceTestHdReflect)
        args.push_back(L"-DFURNACE_TEST_HEMI_DIR_REFLECT");
    else if (m_config.FurnaceTestHhEmit)
        args.push_back(L"-DFURNACE_TEST_HEMI_HEMI_EMIT");

    if (m_config.SamplingStrat == eHaltonOwen)
        args.push_back(L"-DSAMPLING_HALTON_OWEN");
    else if (m_config.SamplingStrat == eHalton)
        args.push_back(L"-DSAMPLING_HALTON");
    else if (m_config.SamplingStrat == eHaltonApple)
        args.push_back(L"-DSAMPLING_HALTON_APPLE");
    else if (m_config.SamplingStrat == eIndependent)
        args.push_back(L"-DSAMPLING_INDEPENDENT");

    if (m_config.LightingModel == eLambertDiff)
        args.push_back(L"-DLIGHTING_LAMB_DIFF");
    else if (m_config.LightingModel == eGlossy)
        args.push_back(L"-DLIGHTING_GLOSSY");
    else if (m_config.LightingModel == eMicrofacet)
        args.push_back(L"-DLIGHTING_MICROFACET");

    if (m_config.DebugPathVisualization)
        args.push_back(L"-DDEBUG_PATH_VISUALIZATION");

    if (m_spectralConfig.SingleLambdaRendering)
        args.push_back(L"-DSINGLE_LAMBDA_RENDERING");

    if (m_spectralConfig.DebugForceWavelengthEnabled)
    {
        args.push_back(L"-DDEBUG_FORCE_WAVELENGTH");
        args.push_back(L"-DDEBUG_FORCED_WAVELENGTH=" + std::to_wstring(m_spectralConfig.DebugForcedWavelength));
    }

    if (m_config.LightingModel == PathTracerLightingModel::eMicrofacet)
    {
        static const std::vector<const WCHAR*> c_mapNdfType = {
            L"-DNDF_TYPE_GGX",
            L"-DNDF_TYPE_BECKMANN"
        };
        args.push_back(c_mapNdfType.at(m_config.NdfType));

        static const std::vector<const WCHAR*> c_mapMaskingType = {
            L"-DMASKING_SMITH",
            L"-DMASKING_VCAVITY"
        };
        args.push_back(c_mapMaskingType.at(m_config.MaskingType));
    }

    m_shaderILD =
    {
        {
            "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
        },
        {
            "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
        }
    };
    const D3D12_INPUT_LAYOUT_DESC ild = {m_shaderILD.data(), static_cast<UINT>(m_shaderILD.size())};

    const wchar_t* psName = isSpectral ? L"Spectral-Tracing/CorePS.hlsl" : L"Path-Tracing/CorePS.hlsl";
    m_shader->InitVsPs(L"FullScreenTriangleVS.hlsl", psName, ild, d3d->GetDevice(), m_rootSig->Get(), false, args);
}
