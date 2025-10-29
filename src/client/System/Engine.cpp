//
// Created by fionaw on 26/10/2025.
//

#include "System/Engine.h"

#include "Helper.h"
#include "imgui.h"
#include "Apps/App.h"
#include "Debug/GPUEventScoped.h"
#include "HWI/D3D.h"
#include "System/Config.h"
#include "System/FileHelper.h"
#include "System/Gui.h"
#include "System/HotReloader.h"
#include "System/Input.h"
#include "System/TextureLoader.h"

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

    {
        GPU_SCOPE(cmdList.Get(), L"GUI");
        RenderGUI();

        const D3D12_CPU_DESCRIPTOR_HANDLE handle = m_d3d->GetRtvHandle();
        cmdList->OMSetRenderTargets(1, &handle, FALSE, nullptr);
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

// TODO: Move somewhere else
#define IM_GUI_INDENTATION 20
void Engine::RenderGUI()
{
    const float startGuiX = Config::GetSystem().WindowAppGuiWidth + Config::GetSystem().RtvWidth;
    Gui::BeginWindow("Engine##xx", ImVec2(startGuiX,0), ImVec2(Config::GetSystem().WindowEngineGuiWidth, Config::GetSystem().RtvHeight));

    ImGui::SeparatorText("Stats##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    const std::string fpsTxt = "FPS (Over 10ms): " + std::to_string(m_fps10ms);
    ImGui::Text("%s", fpsTxt.c_str());

    const std::string fpsTxt5 = "FPS (Over 50ms): " + std::to_string(m_fps50ms);
    ImGui::Text("%s", fpsTxt5.c_str());

    const std::string fpsTxt10 = "FPS (Over 100ms): " + std::to_string(m_fps100ms);
    ImGui::Text("%s", fpsTxt10.c_str());

    static bool pauseFPSQueue = false;

    const bool canUpdateQueue = m_fpsGuiQueue.size() == 0 || m_fps10ms != m_fpsGuiQueue.at(m_fpsGuiQueue.size() - 1);
    if (canUpdateQueue && !pauseFPSQueue)
    {
        m_fpsGuiQueue.push_back(m_fps10ms);
        const float framesPer5Seconds = 5.0 * m_fps10ms;
        const int overflowFrames = m_fpsGuiQueue.size() - framesPer5Seconds;
        if (overflowFrames > 0)
            m_fpsGuiQueue.erase(m_fpsGuiQueue.begin(), m_fpsGuiQueue.begin() + overflowFrames);
    }

    if (ImGui::TreeNode("FPS Plot##xx"))
    {
        ImGui::Indent(IM_GUI_INDENTATION);

        ImGui::PlotLines("##xx", m_fpsGuiQueue.data(), m_fpsGuiQueue.size(), 0, nullptr, 0, 3.4028235E38F, ImVec2(0, 150));

        ImGui::Checkbox("Pause##xx", &pauseFPSQueue);

        ImGui::Unindent(IM_GUI_INDENTATION);
        ImGui::TreePop();
    }

    ImGui::Spacing();
    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Apps##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    const char* curAppName = m_apps.at(m_selectedAppIdx)->GetName();
    if (ImGui::BeginCombo("App##xx", curAppName))
    {
        for (size_t i = 0; i < m_apps.size(); i++)
        {
            const bool isSelected = m_selectedAppIdx == i;
            if (ImGui::Selectable(m_apps[i]->GetName(), isSelected))
            {
                m_selectedAppIdx = i;
                m_changedApps = true;
            }

            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    ImGui::Spacing();
    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Settings##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    ImGui::Checkbox("VSync##xx", &Config::GetSystem().VSyncEnabled);

    ImGui::Spacing();
    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Tools##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

#ifdef _DEBUG
    if (ImGui::Button("Reload All"))
    {
        HotReloader::PendFullReload();
    }
#endif

    Gui::EndWindow();
}

void Engine::CalculateFPS(const double deltaTime)
{
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