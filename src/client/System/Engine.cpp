//
// Created by fionaw on 26/10/2025.
//

#include "System/Engine.h"

#include "Helper.h"
#include "imgui.h"
#include "Apps/App.h"
#include "HWI/D3D.h"
#include "System/Config.h"
#include "System/FileHelper.h"
#include "System/Gui.h"
#include "System/HotReloader.h"
#include "System/Input.h"
#include "System/TextureLoader.h"

Engine::Engine(App* pSample, const HWND hWnd, const UINT windowWidth, const UINT windowHeight)
{
    m_d3d = std::make_unique<D3D>();
    m_d3d->Init(windowWidth, windowHeight);
    TextureLoader::Init(m_d3d.get(), FileHelper::GetAssetsPath() + L"/Shaders");

    pSample->OnInit(m_d3d.get());

    Gui::Init(hWnd, m_d3d->GetDevice(), 3);
}

void Engine::Frame(const HWND hWnd)
{
    Gui::BeginFrame();

    if (App* pSample = reinterpret_cast<App*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)))
    {
        Render(pSample);
    }

    Input::ProgressFrame();

    m_clock.Tick();

#ifdef _DEBUG
    HotReloader::CheckFiles(m_d3d.get());
#endif
}

void Engine::Render(App* pSample) const
{
    ID3D12Resource* rtv = m_d3d->GetCurrRTV();

    const ComPtr<ID3D12GraphicsCommandList> cmdList = m_d3d->GetAvailableCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT);

    const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(rtv, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    cmdList->ResourceBarrier(1, &barrier);

    // ===
    pSample->OnUpdate(m_d3d.get(), cmdList.Get());
    RenderGUI();
    Gui::RenderAllWindows(cmdList.Get());
    // ===

    const auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(rtv, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    cmdList->ResourceBarrier(1, &barrier2);

    V(cmdList->Close());
    m_d3d->ExecuteCommandList(cmdList.Get());
    m_d3d->Present();
}

void Engine::RenderGUI() const
{
    const float startGuiX = Config::GetSystem().WindowAppGuiWidth + Config::GetSystem().RtvWidth;
    Gui::BeginWindow("Engine", ImVec2(startGuiX,0), ImVec2(Config::GetSystem().WindowEngineGuiWidth, Config::GetSystem().RtvHeight));
    ImGui::Text("ENGINE-SIDE GUI");
    Gui::EndWindow();
}
