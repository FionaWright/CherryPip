//
// Created by fionaw on 09/11/2025.
//

#define IM_GUI_INDENTATION 20
#include "imgui.h"
#include "System/Config.h"
#include "System/Engine.h"
#include "System/Gui.h"
#include "System/Input.h"

#ifdef _DEBUG
#include "Debug/HotReloader.h"
#endif

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

    ImGui::Text("Frame Time (ms): %f", m_frameTime * 1000.0);

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

    ImGui::Text("Mouse: (%f, %f)", Input::GetMousePos().x, Input::GetMousePos().y);
    ImGui::Text("Mouse Client: (%f, %f)", Input::GetMousePosClient().x, Input::GetMousePosClient().y);

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
    ImGui::Checkbox("Sync GPU##xx", &Config::GetSystem().ForceSyncCpuGpu);
    ImGui::Checkbox("App Gui Enabled##xx", &Config::GetSystem().AppGuiEnabled);

    ImGui::Spacing();
    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Tools##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

#ifdef _DEBUG
    if (ImGui::Button("Reload All Shaders"))
    {
        HotReloader::PendFullReload();
    }
#endif

    Gui::EndWindow();
}
