#include "Apps/PathTracer/Headers/PathTracer.h"
#include "System/Win32App.h"
#include <dxcapi.h>

#include "imgui.h"
#include "Debug/GPUEventScoped.h"
#include "Headers/Helper.h"
#include "HWI/D3D.h"
#include "System/Config.h"
#include "System/FileHelper.h"
#include "System/Gui.h"
#include "CBV.h"
#include "HWI/BLAS.h"
#include "HWI/Material.h"
#include "System/Input.h"
#include "System/ModelLoaderGLTF.h"

PathTracer::PathTracer()
    : m_AspectRatio(0)
{
}

void PathTracer::OnInit(D3D* d3d)
{
    App::OnInit(d3d);

    if (!d3d->GetRayTracingSupported())
    {
        std::cout << "ERROR: Ray-Tracing not supported!!!" << std::endl;
        return;
    }

    m_AspectRatio = static_cast<float>(Config::GetSystem().RtvWidth) / static_cast<float>(Config::GetSystem().
        RtvHeight);

    constexpr float fov = 60.0f;
    constexpr float nearPlane = 0.1f;
    constexpr float farPlane = 100.0f;
    m_projMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(fov), m_AspectRatio, nearPlane, farPlane);

    m_camera.Init({}, {});

    loadAssets(d3d);
}

void PathTracer::OnUpdate(D3D* d3d, ID3D12GraphicsCommandList* cmdList)
{
    if (!d3d->GetRayTracingSupported())
    {
        std::cout << "ERROR: Ray-Tracing not supported!!!" << std::endl;
        return;
    }

    populateCommandList(d3d, cmdList);
    const bool moved = m_camera.UpdateCamera();
    if (moved)
        m_ptContext.Reset();
}

void PathTracer::loadAssets(D3D* d3d)
{
    ID3D12Device* device = d3d->GetDevice();
    const ComPtr<ID3D12GraphicsCommandList> cmdList = d3d->GetAvailableCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT);

    m_heap.Init(device, 10000);

    m_rootSig = std::make_shared<RootSig>();
    m_rootSig->SmartInit(device, 1, 5, 1);

    m_rootSigDebug = std::make_shared<RootSig>();
    m_rootSigDebug->SmartInit(device, 2, 5, 2);

    // Init Shader/PSO
    {
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

        m_shader = std::make_shared<Shader>();
        m_shader->InitVsPs(L"FullScreenTriangleVS.hlsl", L"Path-Tracing/CorePS.hlsl", ild, device, m_rootSig->Get());
    }

    std::shared_ptr<Texture> tex = std::make_shared<Texture>();
    tex->Init(d3d->GetDevice(), cmdList.Get(), FileHelper::GetAssetTextureFullPath(L"TestTex.png"),
              DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

    tex->Transition(cmdList.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    GLTFLoadArgs args;
    args.DefaultShaderIndex = -1;
    args.DefaultShaderATIndex = -1;
    args.Transform = {};
    args.Transform.SetScale(2.0f);
    ModelLoaderGLTF::LoadSplitModel(d3d, cmdList.Get(), &m_heap, L"Cornell/scene.gltf", args);

    ComPtr<ID3D12Device5> device5;
    V(d3d->GetDevice()->QueryInterface(IID_PPV_ARGS(&device5)));
    ComPtr<ID3D12GraphicsCommandList4> cmdList4;
    V(cmdList->QueryInterface(IID_PPV_ARGS(&cmdList4)));

    std::vector<std::shared_ptr<BLAS>> blasList;
    std::vector<PtMaterialData> materialData;
    for (int i = 0; i < args.OutObjects.size(); i++)
    {
        const Object* object = args.OutObjects[i].get();

        auto blas = std::make_shared<BLAS>();
        blas->Init(device5.Get(), cmdList4.Get(), object->GetModel(), *object->GetTransform());
        blasList.emplace_back(blas);

        const MaterialData* objectMaterialData = object->GetMaterial()->GetData();
        PtMaterialData ptMaterialData;
        ptMaterialData.BaseColorFactor = objectMaterialData->BaseColorFactor;
        ptMaterialData.EmissiveStrength = objectMaterialData->EmmissiveStrength;
        materialData.emplace_back(ptMaterialData);
    }

    auto tlas = std::make_shared<TLAS>();
    tlas->Init(device5.Get(), cmdList4.Get(), blasList);

    m_ptContext.Init(device, cmdList.Get(), tlas, blasList, materialData);

    m_readbackBuffer.Init(d3d, cmdList.Get(), m_ptContext.GetAccumTexture());

    m_material = std::make_shared<Material>();
    m_material->Init(&m_heap);
    m_material->AddCBV(device, &m_heap, sizeof(CbvPathTracing));
    m_ptContext.FillMaterial(device,  m_material.get(), &m_heap);

    m_materialDebug = std::make_shared<Material>();
    m_materialDebug->Init(&m_heap);
    m_materialDebug->AddCBV(device, &m_heap, sizeof(CbvPathTracing));
    m_materialDebug->AddCBV(device, &m_heap, sizeof(CbvPathTracingDebug));
    m_ptContext.FillMaterial(device, m_materialDebug.get(), &m_heap);

    V(cmdList->Close());
    d3d->ExecuteCommandList(cmdList.Get());
    d3d->Flush();
}

void PathTracer::populateCommandList(D3D* d3d, ID3D12GraphicsCommandList* cmdList)
{
    const float fRtvWidth = static_cast<float>(Config::GetSystem().RtvWidth);
    const float fRtvHeight = static_cast<float>(Config::GetSystem().RtvHeight);
    const float fAppGuiWidth = static_cast<float>(Config::GetSystem().WindowAppGuiWidth);

    // Render at offset for ImGui
    const CD3DX12_VIEWPORT viewport(fAppGuiWidth, 0.0f, fRtvWidth, fRtvHeight);
    const CD3DX12_RECT scissorRect(Config::GetSystem().WindowAppGuiWidth, 0,
                                   Config::GetSystem().RtvWidth + Config::GetSystem().WindowAppGuiWidth,
                                   Config::GetSystem().RtvHeight);

    cmdList->RSSetViewports(1, &viewport);
    cmdList->RSSetScissorRects(1, &scissorRect);

    const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = d3d->GetRtvHandle();
    cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
    cmdList->ClearRenderTargetView(d3d->GetRtvHandle(), Config::GetSystem().RtvClearColor, 1, &scissorRect);

    m_heap.SetHeap(cmdList);

    const bool debugMode = m_ptConfig.Mode == eOutputBuffer;

    ID3D12RootSignature* rootSig = debugMode ? m_rootSigDebug->Get() : m_rootSig->Get();
    const Material* mat = debugMode ? m_materialDebug.get() : m_material.get();
    const int debugBufferIdx = debugMode ? m_ptConfig.DebugBufferIdx : -1;

    ID3D12PipelineState* pso = nullptr;
    switch (m_ptConfig.Mode)
    {
    case eStandard:
        pso = m_shader->GetPSO();
        break;
    case eOutputBuffer:
        if (!m_shaderDebug)
        {
            const D3D12_INPUT_LAYOUT_DESC ild = {m_shaderILD.data(), static_cast<UINT>(m_shaderILD.size())};
            m_shaderDebug = std::make_shared<Shader>();
            m_shaderDebug->InitVsPs(L"FullScreenTriangleVS.hlsl", L"Path-Tracing/CoreDebugPS.hlsl", ild, d3d->GetDevice(), m_rootSigDebug->Get());
        }
        pso = m_shaderDebug->GetPSO();
        break;
    case eFurnaceTestClassic:
        if (!m_shaderFurnaceClassic)
        {
            const D3D12_INPUT_LAYOUT_DESC ild = {m_shaderILD.data(), static_cast<UINT>(m_shaderILD.size())};
            m_shaderFurnaceClassic = std::make_shared<Shader>();
            m_shaderFurnaceClassic->InitVsPs(L"FullScreenTriangleVS.hlsl", L"Path-Tracing/CoreFurnaceTestClassicPS.hlsl", ild, d3d->GetDevice(), m_rootSig->Get());
        }
        pso = m_shaderFurnaceClassic->GetPSO();
        break;
    case eFurnaceTestEmissive:
        if (!m_shaderFurnaceEmissive)
        {
            const D3D12_INPUT_LAYOUT_DESC ild = {m_shaderILD.data(), static_cast<UINT>(m_shaderILD.size())};
            m_shaderFurnaceEmissive = std::make_shared<Shader>();
            m_shaderFurnaceEmissive->InitVsPs(L"FullScreenTriangleVS.hlsl", L"Path-Tracing/CoreFurnaceTestEmissivePS.hlsl", ild, d3d->GetDevice(), m_rootSig->Get());
        }
        pso = m_shaderFurnaceEmissive->GetPSO();
        break;
    }

    m_ptContext.Render(cmdList, rootSig, pso, &m_camera.GetCamera(), mat, m_projMatrix, m_ptConfig, debugBufferIdx);

    if (m_ptConfig.ReadbackEnabled)
    {
        if (Input::IsMouseLeftDown())
        {
            const XMFLOAT2 mousePos = Input::GetMousePos();
            const uint32_t minX = Config::GetSystem().WindowAppGuiWidth;
            const uint32_t maxX = Config::GetSystem().WindowAppGuiWidth + Config::GetSystem().RtvWidth;
            if (mousePos.x >= minX && mousePos.x < maxX)
                m_mousePosOnClick = { mousePos.x - minX, mousePos.y };
        }

        const bool validMousePos = m_mousePosOnClick.x != -1 && m_mousePosOnClick.y != -1;
        if (validMousePos && !(m_ptConfig.ReadbackEveryFrame && !m_readingBackEveryFrame))
        {
            m_readbackBuffer.Readback(d3d);
            const uint8_t* byteData = m_readbackBuffer.GetData();
            const auto* rgbaData = reinterpret_cast<const Rgba8*>(byteData);
            const auto pixelIdx = static_cast<size_t>(m_mousePosOnClick.y * static_cast<float>(Config::GetSystem().RtvWidth) + m_mousePosOnClick.x);
            const Rgba8 pixelData = rgbaData[pixelIdx];
            m_readbackRgbaData.push_back(pixelData);

            if (!m_readingBackEveryFrame)
                m_mousePosOnClick = { -1, -1};
        }
    }

    GUI();
}

#define IM_GUI_INDENTATION 20 // Temp
void PathTracer::GUI()
{
    Gui::BeginWindow("PathTracer", ImVec2(0, 0), ImVec2(Config::GetSystem().WindowAppGuiWidth, Config::GetSystem().RtvHeight));

    ImGui::SeparatorText("Stats##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    ImGui::Text("%s%i", "Frame Index: ", m_ptContext.GetFrameNum());
    ImGui::Text("%s%i", "Total SPP: ", m_ptContext.GetFrameNum() * m_ptConfig.SPP);

    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Settings##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    bool ptNeedsReset = false;

    ptNeedsReset |= ImGui::Checkbox("Accumulation Enabled##xx", &m_ptConfig.AccumulationEnabled);
    ptNeedsReset |= ImGui::Checkbox("Jitter Enabled##xx", &m_ptConfig.JitterEnabled);

    int spp = static_cast<int>(m_ptConfig.SPP);
    ptNeedsReset |= ImGui::DragInt("SPP##xx", &spp, 1, 1, 256);
    m_ptConfig.SPP = static_cast<uint32_t>(spp);

    int bounces = static_cast<int>(m_ptConfig.NumBounces);
    ptNeedsReset |= ImGui::DragInt("Ray Bounces##xx", &bounces, 1, 0, 256);
    m_ptConfig.NumBounces = static_cast<uint32_t>(bounces);

    int maxFrame = static_cast<int>(m_ptConfig.MaxFrameNum);
    ptNeedsReset |= ImGui::InputInt("Max Frames##xx", &maxFrame);
    m_ptConfig.MaxFrameNum = static_cast<uint32_t>(maxFrame);

    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Tools##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    ptNeedsReset |= ImGui::Button("Reset PathTracer##xx");

    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Debug##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    const bool prevReadbackEnabled = m_ptConfig.ReadbackEnabled;
    ImGui::Checkbox("Enable Readback##xx", &m_ptConfig.ReadbackEnabled);

    if (m_ptConfig.ReadbackEnabled)
    {
        ImGui::Indent(IM_GUI_INDENTATION);

        if (!prevReadbackEnabled)
            m_readbackRgbaData.clear();

        ImGui::Checkbox("Readback Every Frame##xx", &m_ptConfig.ReadbackEveryFrame);
        if (m_ptConfig.ReadbackEveryFrame)
            ImGui::Text("%s", "(Starts on PathTracer Reset!)");

        if (!m_readbackRgbaData.empty() && ImGui::TreeNode("Readback Data##xx"))
        {
            ImGui::Indent(IM_GUI_INDENTATION);

            for (int i = 0; i < m_readbackRgbaData.size(); i++)
            {
                const float r = static_cast<float>(m_readbackRgbaData[i].r) / 255.0f;
                const float g = static_cast<float>(m_readbackRgbaData[i].g) / 255.0f;
                const float b = static_cast<float>(m_readbackRgbaData[i].b) / 255.0f;
                const float a = static_cast<float>(m_readbackRgbaData[i].a) / 255.0f;
                ImGui::Text("%i: (%.3f, %.3f, %.3f, %.3f)", i, r, g, b, a);
            }

            ImGui::Unindent(IM_GUI_INDENTATION);
            ImGui::TreePop();
        }

        ImGui::Unindent(IM_GUI_INDENTATION);
    }

    static const std::vector<const char*> c_debugBufferStrMap = {
        "Normals",
        "Base Color",
        "HitPos",
        "First Bounce Direction",
        "Miss / Hit",
        "Hit Dist Ray 0",
        "Hit Dist Ray 1",
        "Material ID",
    };

    static int e = 0;
    ImGui::Text("%s", "Path Tracer Mode:");
    ImGui::Indent(IM_GUI_INDENTATION);
    ptNeedsReset |= ImGui::RadioButton("Standard", &e, 0);
    ptNeedsReset |= ImGui::RadioButton("Debug Buffer", &e, 1);
    if (m_ptConfig.Mode == eOutputBuffer)
    {
        const char* curSelection = c_debugBufferStrMap.at(m_ptConfig.DebugBufferIdx);
        if (ImGui::BeginCombo("Debug Buffer##xx", curSelection))
        {
            for (size_t i = 0; i < c_debugBufferStrMap.size(); i++)
            {
                const bool isSelected = m_ptConfig.DebugBufferIdx == i;
                if (ImGui::Selectable(c_debugBufferStrMap.at(i), isSelected))
                {
                    m_ptConfig.DebugBufferIdx = static_cast<DebugBuffer>(i);
                    ptNeedsReset = true;
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }
    }
    ptNeedsReset |= ImGui::RadioButton("Furnace Test (Classic)", &e, 2);
    ptNeedsReset |= ImGui::RadioButton("Furnace Test (Emissive)", &e, 3);
    ImGui::Unindent(IM_GUI_INDENTATION);
    m_ptConfig.Mode = static_cast<PathTracerMode>(e);

    if (ptNeedsReset)
    {
        m_ptContext.Reset();
        m_readbackRgbaData.clear();
    }

    if (ptNeedsReset && m_ptConfig.ReadbackEveryFrame)
        m_readingBackEveryFrame = true;
    else if (!m_ptConfig.ReadbackEveryFrame)
        m_readingBackEveryFrame = false;

    ImGui::Unindent(IM_GUI_INDENTATION);

    Gui::EndWindow();
}
