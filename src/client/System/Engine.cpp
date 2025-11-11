//
// Created by fionaw on 26/10/2025.
//

#include "System/Engine.h"

#include "Helper.h"
#include "imgui.h"
#include "../../../Headers/client/System/App.h"
#include "Debug/GPUEventScoped.h"
#include "HWI/D3D.h"
#include "System/Config.h"
#include "System/FileHelper.h"
#include "System/Gui.h"
#include "System/Input.h"
#include "System/TextureLoader.h"

#ifdef _DEBUG
#include "Debug/HotReloader.h"
#endif

Engine::Engine(const std::vector<App*>& apps, const HWND hWnd, const UINT windowWidth, const UINT windowHeight)
{
    assert(apps.size() > 0);

    m_d3d = std::make_unique<D3D>();
    m_d3d->Init(windowWidth, windowHeight);
    TextureLoader::Init(m_d3d.get(), FileHelper::GetAssetsPath() + L"/Shaders");

    m_selectedAppIdx = Config::GetSystem().DefaultAppIdx;

    m_apps = apps;
    m_apps.at(m_selectedAppIdx)->OnInit(m_d3d.get());

    Gui::Init(hWnd, m_d3d->GetDevice(), 3);
}

void Engine::Frame()
{
    Gui::BeginFrame();

    const TimeArgs timeArgs = m_clock.GetTimeArgs();
    CalculateFPS(timeArgs.ElapsedTime);

    Render();

    Input::ProgressFrame();

    m_clock.Tick();

#ifdef _DEBUG
    HotReloader::CheckFiles(m_d3d.get());
#endif
}

void Engine::Render()
{
    const ComPtr<ID3D12GraphicsCommandList> cmdList = m_d3d->GetAvailableCmdList(D3D12_COMMAND_LIST_TYPE_DIRECT);

    D12Resource* rtv = m_d3d->GetRtv();
    rtv->Transition(cmdList.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    m_apps.at(m_selectedAppIdx)->OnUpdate(m_d3d.get(), cmdList.Get());
    m_apps.at(m_selectedAppIdx)->RenderGUI();

    {
        GPU_SCOPE(cmdList.Get(), L"GUI");
        RenderGUI();

        const D3D12_CPU_DESCRIPTOR_HANDLE handle = m_d3d->GetRtvHandle();
        cmdList->OMSetRenderTargets(1, &handle, FALSE, nullptr);
        rtv->Transition(cmdList.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
        Gui::RenderAllWindows(cmdList.Get());
    }

    rtv->Transition(cmdList.Get(), D3D12_RESOURCE_STATE_PRESENT);

    V(cmdList->Close());
    m_d3d->ExecuteCommandList(cmdList.Get());
    m_d3d->Present();

    if (m_changedApps)
    {
        m_d3d->Flush();
        if (!m_apps.at(m_selectedAppIdx)->GetIsInitialized())
            m_apps.at(m_selectedAppIdx)->OnInit(m_d3d.get());
        m_changedApps = false;
    }
}

void Engine::CalculateFPS(const double deltaTime)
{
    m_frameTime = deltaTime;

    m_fpsTimeSinceUpdate10ms += deltaTime;
    m_fpsTimeSinceUpdate50ms += deltaTime;
    m_fpsTimeSinceUpdate100ms += deltaTime;
    m_fpsFramesSinceUpdate10ms++;
    m_fpsFramesSinceUpdate50ms++;
    m_fpsFramesSinceUpdate100ms++;

    if (m_fpsTimeSinceUpdate10ms > 0.1)
    {
        m_fps10ms = m_fpsFramesSinceUpdate10ms / m_fpsTimeSinceUpdate10ms;

        m_fpsFramesSinceUpdate10ms = 0;
        m_fpsTimeSinceUpdate10ms = 0.0;
    }

    if (m_fpsTimeSinceUpdate50ms > 0.5)
    {
        m_fps50ms = m_fpsFramesSinceUpdate50ms / m_fpsTimeSinceUpdate50ms;

        m_fpsFramesSinceUpdate50ms = 0;
        m_fpsTimeSinceUpdate50ms = 0.0;
    }

    if (m_fpsTimeSinceUpdate100ms > 1.0)
    {
        m_fps100ms = m_fpsFramesSinceUpdate100ms / m_fpsTimeSinceUpdate100ms;

        m_fpsFramesSinceUpdate100ms = 0;
        m_fpsTimeSinceUpdate100ms = 0.0;
    }
}