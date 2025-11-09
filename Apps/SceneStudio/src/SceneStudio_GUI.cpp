//
// Created by fionaw on 09/11/2025.
//

#include "ThirdParty/imgui/imgui.h"
#include "Apps/SceneStudio/Headers/SceneStudio.h"
#include "Render/Scene.h"
#include "System/Gui.h"

#ifdef _DEBUG
#include "Debug/PythonExecutor.h"
#endif

#define IM_GUI_INDENTATION 20 // Temp

void SceneStudio::GUI()
{
    Gui::BeginWindow("SceneStudio", ImVec2(0, 0),
                     ImVec2(Config::GetSystem().WindowAppGuiWidth, Config::GetSystem().RtvHeight));

    static int e = m_studioConfig.Backend;
    ImGui::SeparatorText("Render Backend##xx");
    ImGui::Indent(IM_GUI_INDENTATION);
    ImGui::RadioButton("Forward", &e, 0); ImGui::SameLine();
    ImGui::RadioButton("Path Tracer", &e, 1);
    ImGui::Unindent(IM_GUI_INDENTATION);
    m_studioConfig.Backend = static_cast<RenderBackend>(e);

    switch (m_studioConfig.Backend)
    {
    case eForward:
        GuiRaster();
        break;
    case ePathTracer:
        GuiPathTracer();
        break;
    default:
        break;
    }

    Gui::EndWindow();
}

void SceneStudio::GuiPathTracer()
{
    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Stats##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    ImGui::Text("%s%i", "Frame Index: ", m_ptContext.GetFrameNum());
    ImGui::Text("%s%i", "Total SPP: ", m_ptContext.GetFrameNum() * m_studioConfig.PT.SPP);

    bool ptNeedsReset = false;

    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Scene##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    const char* curName = m_scenes.at(m_currentScene)->GetName();
    if (ImGui::BeginCombo("Scene##xx", curName))
    {
        for (size_t i = 0; i < m_scenes.size(); i++)
        {
            const bool isSelected = m_currentScene == i;
            if (ImGui::Selectable(m_scenes.at(i)->GetName(), isSelected))
            {
                m_currentScene = i;
                m_sceneDirty = true;
                ptNeedsReset = true;
            }

            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Settings##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    ptNeedsReset |= ImGui::Checkbox("Accumulation Enabled##xx", &m_studioConfig.PT.AccumulationEnabled);
    ptNeedsReset |= ImGui::Checkbox("Jitter Enabled##xx", &m_studioConfig.PT.JitterEnabled);

    int spp = static_cast<int>(m_studioConfig.PT.SPP);
    ptNeedsReset |= ImGui::DragInt("SPP##xx", &spp, 1, 1, 256);
    m_studioConfig.PT.SPP = static_cast<uint32_t>(spp);

    int bounces = static_cast<int>(m_studioConfig.PT.NumBounces);
    ptNeedsReset |= ImGui::DragInt("Ray Bounces##xx", &bounces, 1, 0, 256);
    m_studioConfig.PT.NumBounces = static_cast<uint32_t>(bounces);

    int maxFrame = static_cast<int>(m_studioConfig.PT.MaxFrameNum);
    ptNeedsReset |= ImGui::InputInt("Max Frames##xx", &maxFrame);
    m_studioConfig.PT.MaxFrameNum = static_cast<uint32_t>(maxFrame);

    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Tools##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    ptNeedsReset |= ImGui::Button("Reset PathTracer##xx");

#ifdef _DEBUG
    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Debug##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    const bool prevReadbackEnabled = m_studioConfig.PT.ReadbackEnabled;
    ImGui::Checkbox("Enable Readback##xx", &m_studioConfig.PT.ReadbackEnabled);

    if (m_studioConfig.PT.ReadbackEnabled)
    {
        m_readbackManager.GUI(prevReadbackEnabled, m_studioConfig.PT.ReadbackEveryFrame);
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
        "RNG",
        "Self-Intersection",
        "NaN",
    };

    static int e = 0;
    ImGui::Text("%s", "Path Tracer Mode:");
    ImGui::Indent(IM_GUI_INDENTATION);
    ptNeedsReset |= ImGui::RadioButton("Standard", &e, 0);
    ptNeedsReset |= ImGui::RadioButton("Debug Buffer", &e, 1);
    if (m_studioConfig.PT.Mode == eOutputBuffer)
    {
        const char* curSelection = c_debugBufferStrMap.at(m_studioConfig.PT.DebugBufferIdx);
        if (ImGui::BeginCombo("Debug Buffer##xx", curSelection))
        {
            for (size_t i = 0; i < c_debugBufferStrMap.size(); i++)
            {
                const bool isSelected = m_studioConfig.PT.DebugBufferIdx == i;
                if (ImGui::Selectable(c_debugBufferStrMap.at(i), isSelected))
                {
                    m_studioConfig.PT.DebugBufferIdx = static_cast<DebugBuffer>(i);
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
    m_studioConfig.PT.Mode = static_cast<PathTracerMode>(e);

    if (ptNeedsReset && m_studioConfig.PT.ReadbackEveryFrame)
        m_readbackManager.SetInReadbackProcess(true);
    else if (!m_studioConfig.PT.ReadbackEveryFrame)
        m_readbackManager.SetInReadbackProcess(false);

    ImGui::Unindent(IM_GUI_INDENTATION);
#endif

    if (ptNeedsReset)
    {
        m_ptContext.Reset();
#ifdef _DEBUG
        m_readbackManager.ClearReadbackData();
#endif
    }
}

void SceneStudio::GuiRaster()
{
    ImGui::Text("%s", "Raster Backend, settings coming soon!");
}
