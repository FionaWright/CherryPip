//
// Created by fionaw on 09/11/2025.
//

#include "System/pch.h"
#include "Apps/SceneStudio/Headers/ReadbackManager.h"

#include "CBV.h"
#include "ThirdParty/imgui/imgui.h"
#include "Debug/PythonExecutor.h"
#include "HWI/Material.h"
#include "HWI/RootSig.h"
#include "HWI/Shader.h"
#include "Render/TextureRTV.h"
#include "System/Input.h"

void ReadbackManager::Init(const D3D* d3d, Heap* heap, TextureRTV* ptOut)
{
    ID3D12Device* device = d3d->GetDevice();

    m_rootSigReadbackHighlight = std::make_shared<RootSig>();
    m_rootSigReadbackHighlight->SmartInit(device, 1, 0, 1);

    m_shaderReadbackHighlight = std::make_shared<Shader>();
    m_shaderReadbackHighlight->InitCs(L"Compute/DebugHighlightPixelCS.hlsl", device, m_rootSigReadbackHighlight->Get());

    m_readbackBuffer.Init(d3d, Config::GetSystem().RtvWidth, Config::GetSystem().RtvHeight);

    m_materialReadbackHighlight = std::make_shared<Material>();
    m_materialReadbackHighlight->Init(heap);
    m_materialReadbackHighlight->AddCBV(device, heap, sizeof(CbvHighlightPixel), "CBV Highlight Pixel");
    m_materialReadbackHighlight->AddUAV(device, heap, ptOut->GetResource(), ptOut->GetD12Resource()->GetDesc().Format);
}

void ReadbackManager::ReadbackPass(D3D* d3d, ID3D12GraphicsCommandList* cmdList, TextureRTV* ptOut, const bool readbackEveryFrame)
{
    if (Input::IsMouseLeftDown())
    {
        const XMFLOAT2 mousePos = Input::GetMousePos();
        const uint32_t minX = Config::GetSystem().WindowAppGuiWidth;
        const uint32_t maxX = Config::GetSystem().WindowAppGuiWidth + Config::GetSystem().RtvWidth;
        if (mousePos.x >= minX && mousePos.x < maxX)
        {
            m_mousePosOnClick = {mousePos.x - minX, mousePos.y};
            m_finishedReadingBack = false;
        }
    }

    const bool validMousePos = m_mousePosOnClick.x != -1 && m_mousePosOnClick.y != -1;
    if (!validMousePos)
        return;

    const uint32_t px = static_cast<uint32_t>(m_mousePosOnClick.x);
    const uint32_t py = static_cast<uint32_t>(m_mousePosOnClick.y);

    // Post-Pass: Highlight Selected Pixel
    {
        ptOut->GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        cmdList->SetComputeRootSignature(m_rootSigReadbackHighlight->Get());

        CbvHighlightPixel highlightPixel;
        highlightPixel.SelectedPixelCoords = { px, py };
        m_materialReadbackHighlight->UpdateCBV(0, &highlightPixel);

        m_materialReadbackHighlight->SetDescriptorTables(cmdList, true);

        cmdList->SetPipelineState(m_shaderReadbackHighlight->GetPSO());
        cmdList->Dispatch(1, 1, 1);

        ptOut->GetD12Resource()->Transition(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }

    if (readbackEveryFrame && !m_inReadbackEveryFrameProcess)
        return;

    if (m_finishedReadingBack)
        return;

    m_readbackBuffer.Readback(d3d, ptOut->GetD12Resource());
    const std::vector<uint8_t>& byteData = m_readbackBuffer.GetData();
    const size_t pixelIdx = py * Config::GetSystem().RtvWidth + px;
    const Rgba8* rgbaData = reinterpret_cast<const Rgba8*>(byteData.data());
    const Rgba8 pixelData = rgbaData[pixelIdx];
    m_readbackRgbaData.push_back(pixelData);

    if (!m_inReadbackEveryFrameProcess)
        m_finishedReadingBack = true;
}

#define IM_GUI_INDENTATION 20 // Temp

void ReadbackManager::GUI(const bool prevReadbackEnabled, bool& readbackEveryFrame)
{
    ImGui::Indent(IM_GUI_INDENTATION);

    if (!prevReadbackEnabled || ImGui::Button("Clear##xx"))
        m_readbackRgbaData.clear();

    ImGui::Checkbox("Readback Every Frame##xx", &readbackEveryFrame);
    if (readbackEveryFrame)
        ImGui::Text("%s", "(Starts on PathTracer Reset!)");

    const std::string label = "Readback Data (" + std::to_string(m_readbackRgbaData.size()) + ")##xx";
    if (ImGui::TreeNode(label.c_str()))
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

    if (ImGui::Button("Plot data (Execute Py)##xx"))
    {
        const std::string csvPath = std::string(SOURCE_DIR) + "/Data/ReadbackData.csv";
        std::ofstream f(csvPath);
        f.clear();
        f << "data" << "\n";
        for (const Rgba8 rgba : m_readbackRgbaData)
            f << std::to_string(rgba.r) << "\n";
        f.close();

        const std::vector<const char*> args = {
            csvPath.c_str(),
            "--show"
        };
        PythonExecutor::ExecutePython("Histogram.py", args);
    }

    ImGui::Unindent(IM_GUI_INDENTATION);
}
