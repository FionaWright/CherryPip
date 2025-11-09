//
// Created by fionaw on 09/11/2025.
//

#include "imgui.h"
#include "Apps/PathTracer/Headers/PathTracer.h"
#include "System/Gui.h"

#ifdef _DEBUG
#include "Debug/PythonExecutor.h"
#endif

#define IM_GUI_INDENTATION 20 // Temp

void PathTracer::GUI()
{
    Gui::BeginWindow("PathTracer", ImVec2(0, 0),
                     ImVec2(Config::GetSystem().WindowAppGuiWidth, Config::GetSystem().RtvHeight));

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

#ifdef _DEBUG
    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Debug##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    const bool prevReadbackEnabled = m_ptConfig.ReadbackEnabled;
    ImGui::Checkbox("Enable Readback##xx", &m_ptConfig.ReadbackEnabled);

    if (m_ptConfig.ReadbackEnabled)
    {
        m_readbackManager.GUI(prevReadbackEnabled, m_ptConfig.ReadbackEveryFrame);
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

    if (ptNeedsReset && m_ptConfig.ReadbackEveryFrame)
        m_readbackManager.SetInReadbackProcess(true);
    else if (!m_ptConfig.ReadbackEveryFrame)
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

    Gui::EndWindow();
}