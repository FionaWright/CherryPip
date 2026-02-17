//
// Created by fionaw on 09/11/2025.
//

#include "System/pch.h"
#include <bit>

#include "Helper.h"
#include "ThirdParty/imgui/imgui.h"
#include "Apps/SceneStudio/Headers/SceneStudio.h"
#include "Render/Object.h"
#include "Render/Scene.h"
#include "System/Gui.h"

#include "System/ImGuiUtils.h"

#ifdef _DEBUG
#include "Debug/PythonExecutor.h"
#endif

#define IM_GUI_INDENTATION 20 // Temp

void SceneStudio::RenderGUI()
{
    Gui::BeginWindow("SceneStudio", ImVec2(0, 0),
                     ImVec2(Config::GetSystem().WindowAppGuiWidth, Config::GetSystem().RtvHeight));

    if (ImGui::BeginTabBar("Scene Studio GUI"))
    {
        if (ImGui::BeginTabItem("Core"))
        {
            guiMain();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Path-Tracer"))
        {
            guiPathTracer();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Raster"))
        {
            guiRaster();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Scene"))
        {
            guiScene();
            ImGui::EndTabItem();
        }

#ifdef _DEBUG
        if (Config::GetSystem().DebugHeapEnabled && ImGui::BeginTabItem("Heap"))
        {
            guiHeapDebug();
            ImGui::EndTabItem();
        }
#endif

        ImGui::EndTabBar();
    }

    m_pathTracerDirty |= m_shaderDirty;

#ifdef _DEBUG
    if (m_pathTracerDirty && m_studioConfig.PT.ReadbackEveryFrame)
        m_readbackManager.SetInReadbackProcess(true);
    else if (!m_studioConfig.PT.ReadbackEveryFrame)
        m_readbackManager.SetInReadbackProcess(false);
#endif

    if (m_pathTracerDirty)
    {
        m_ptContext.Reset();
        m_pathTracerDirty = false;
#ifdef _DEBUG
        m_readbackManager.ClearReadbackData();
#endif
    }

    Gui::EndWindow();
}

void SceneStudio::guiPathTracer()
{
    ImGui::SeparatorText("Stats##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    ImGui::Text("%s%i", "Frame Index: ", m_ptContext.GetFrameNum());
    ImGui::Text("%s%i", "Total SPP: ", m_ptContext.GetFrameNum() * m_studioConfig.PT.SPP);

    ImGui::Spacing();
    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Settings##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    m_pathTracerDirty |= ImGui::Checkbox("Accumulation Enabled##xx", &m_studioConfig.PT.AccumulationEnabled);
    m_shaderDirty |= ImGui::Checkbox("Jitter Enabled##xx", &m_studioConfig.PT.JitterEnabled);
    m_shaderDirty |= ImGui::Checkbox("Russian Roulette##xx", &m_studioConfig.PT.RussianRouletteEnabled);

    if (m_studioConfig.PT.RussianRouletteEnabled)
        m_pathTracerDirty |= ImGuiUtils::FwInputUInt("Russian Roulette Min Bounces##xx", &m_studioConfig.PT.RussianRouletteMinBounces);

    int spp = static_cast<int>(m_studioConfig.PT.SPP);
    m_pathTracerDirty |= ImGuiUtils::FwDragInt("SPP##xx", &spp, 1, 1, 256);
    m_studioConfig.PT.SPP = static_cast<uint32_t>(spp);

    int bounces = static_cast<int>(m_studioConfig.PT.NumBounces);
    m_pathTracerDirty |= ImGuiUtils::FwDragInt("Ray Bounces##xx", &bounces, 1, 0, 256);
    m_studioConfig.PT.NumBounces = static_cast<uint32_t>(bounces);

    const uint32_t prevMaxFrames = m_studioConfig.PT.MaxFrameNum;
    if (ImGuiUtils::FwInputUInt("Max Frames##xx", &m_studioConfig.PT.MaxFrameNum))
        m_pathTracerDirty |= prevMaxFrames > m_studioConfig.PT.MaxFrameNum || prevMaxFrames == 0;

    m_shaderDirty |= ImGui::Checkbox("Dir Light Enabled##xx", &m_studioConfig.PT.DirLightEnabled);
    m_pathTracerDirty |= ImGuiUtils::FwInputFloat("Dir Light Radius (R)##xx", &m_studioConfig.PT.DirLightCosAngularRadius);

    m_shaderDirty |= ImGui::Checkbox("Normal Maps Enabled##xx", &m_studioConfig.PT.NormalMapsEnabled);
    m_shaderDirty |= ImGui::Checkbox("Alpha Testing Enabled##xx", &m_studioConfig.PT.AlphaTestingEnabled);

    m_shaderDirty |= ImGuiUtils::FwInputFloat("Firefly Threshold", &m_studioConfig.PT.FireflyThreshold);

    m_shaderDirty |= ImGui::Checkbox("Depth Of Field Enabled", &m_studioConfig.PT.DepthOfFieldEnabled);
    if (m_studioConfig.PT.DepthOfFieldEnabled)
    {
        ImGui::Indent(IM_GUI_INDENTATION);
        m_shaderDirty |= ImGuiUtils::FwDragFloat("Focal Distance", &m_studioConfig.PT.DofFocalDist, 0.03f);
        m_shaderDirty |= ImGuiUtils::FwDragFloat("Lens Radius", &m_studioConfig.PT.DofLensRadius, 0.01f);
        ImGui::Unindent(IM_GUI_INDENTATION);
    }

    static int e = m_studioConfig.PT.LightingModel;
    ImGui::Text("%s", "Lighting Model:");
    int idx = 0;
    ImGui::Indent(IM_GUI_INDENTATION);
    m_shaderDirty |= ImGui::RadioButton("Lambert", &e, idx++);
    m_shaderDirty |= ImGui::RadioButton("Glossy", &e, idx++);
    m_shaderDirty |= ImGui::RadioButton("Microfacet", &e, idx++);
    ImGui::Unindent(IM_GUI_INDENTATION);
    m_studioConfig.PT.LightingModel = static_cast<PathTracerLightingModel>(e);

    ImGui::Text("%s", "RNG Sampling Strategy:");
    static int e3 = m_studioConfig.PT.SamplingStrat;
    idx = 0;
    ImGui::Indent(IM_GUI_INDENTATION);
    m_shaderDirty |= ImGui::RadioButton("Independent", &e3, idx++);
    m_shaderDirty |= ImGui::RadioButton("Halton", &e3, idx++);
    m_shaderDirty |= ImGui::RadioButton("Apple Halton", &e3, idx++);
    m_shaderDirty |= ImGui::RadioButton("Owen Scrambled Halton", &e3, idx++);
    ImGui::Unindent(IM_GUI_INDENTATION);
    m_studioConfig.PT.SamplingStrat = static_cast<PathTracerSamplingStrategy>(e3);

    if (m_studioConfig.PT.LightingModel == eMicrofacet)
    {
        ImGui::Text("%s", "Microfacet Normal Distribution Function:");
        static int e2 = m_studioConfig.PT.NdfType;
        idx = 0;
        ImGui::Indent(IM_GUI_INDENTATION);
        m_shaderDirty |= ImGui::RadioButton("GGX", &e2, idx++);
        m_shaderDirty |= ImGui::RadioButton("Beckmann", &e2, idx++);
        ImGui::Unindent(IM_GUI_INDENTATION);
        m_studioConfig.PT.NdfType = static_cast<MicrofacetNdfType>(e2);

        m_shaderDirty |= ImGui::Checkbox("Anisotropy Enabled", &m_studioConfig.PT.AnisotropyEnabled);
        m_shaderDirty |= ImGui::Checkbox("Sample Visible Normals", &m_studioConfig.PT.SampleVisibleNormals);
    }

    if (m_studioConfig.PT.LightingModel != eLambertDiff)
        m_shaderDirty |= ImGui::Checkbox("Glass Model Enabled##xx", &m_studioConfig.PT.GlassModelEnabled);
    m_shaderDirty |= ImGui::Checkbox("Importance Sampling Enabled##xx", &m_studioConfig.PT.ImportanceSamplingEnabled);

    ImGui::Spacing();
    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Tools##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    m_pathTracerDirty |= ImGui::Button("Reset PathTracer##xx");

#ifdef _DEBUG
    ImGui::Spacing();
    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Debug##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    const bool prevReadbackEnabled = m_studioConfig.PT.ReadbackEnabled;
    ImGui::Checkbox("Enable Readback##xx", &m_studioConfig.PT.ReadbackEnabled);

    if (m_studioConfig.PT.ReadbackEnabled)
    {
        m_readbackManager.GUI(prevReadbackEnabled, m_studioConfig.PT.ReadbackEveryFrame);
    }

    ImGui::Checkbox("Path Visualization Enabled", &m_studioConfig.PT.DebugPathVisualization);
    if (m_studioConfig.PT.DebugPathVisualization)
    {
        ImGui::Indent(IM_GUI_INDENTATION);

        const bool validMousePos = m_mousePosOnClick.x != -1 && m_mousePosOnClick.y != -1;
        if (validMousePos)
        {
            m_takePathVisualizationSnapshot |= ImGui::Button("Take Snapshot##xxx");
            m_shaderDirty |= m_takePathVisualizationSnapshot;
        }
        else
            ImGui::Text("Please select a pixel");

        ImGui::Unindent(IM_GUI_INDENTATION);
    }

    static const std::vector<const char*> c_debugBufferStrMap = {
        "Normals (Shaded)",
        "Normals (Geometric)",
        "Base Color",
        "Ray Direction",
        "HitPos",
        "First Bounce Direction",
        "Miss / Hit",
        "Hit Dist Ray 0",
        "Hit Dist Ray 1",
        "Material ID",
        "RNG",
        "RNG 2D",
        "Self-Intersection",
        "NaN",
        "Albedo Alpha",
        "Firefly Threshold Hit",
        "Roughness",
        "Metalness",
        "Microfacet: Tangent",
        "Microfacet: Binormal",
        "Microfacet: View Vector (WS)",
        "Microfacet: View Vector (SS)",
        "Microfacet: Light Vector (SS)",
        "Microfacet: Half Vector (SS)",
        "Microfacet: Aniso Dir (AS)",
        "Microfacet: Aniso Strength",
        "Microfacet: Alpha",
        "Microfacet: Aniso Alpha",
        "Microfacet: Specular Probability",
        "Microfacet: D",
        "Microfacet: F",
        "Microfacet: G",
        "Microfacet: BRDF Diffuse",
        "Microfacet: BRDF Specular",
        "Microfacet: PDF Diffuse",
        "Microfacet: PDF Specular",
    };

    m_shaderDirty |= ImGui::Checkbox("Info Output Enabled##xx", &m_studioConfig.PT.DebugInfoOutputEnabled);
    if (m_studioConfig.PT.DebugInfoOutputEnabled)
    {
        ImGui::Indent(IM_GUI_INDENTATION);
        const char* curSelection = c_debugBufferStrMap.at(static_cast<uint32_t>(m_studioConfig.PT.DebugInfoOutputMode));
        if (ImGuiUtils::BeginComboWithTooltip("Info Output Mode##xx", curSelection))
        {
            for (size_t i = 0; i < c_debugBufferStrMap.size(); i++)
            {
                const bool isSelected = static_cast<uint32_t>(m_studioConfig.PT.DebugInfoOutputMode) == i;
                if (ImGui::Selectable(c_debugBufferStrMap.at(i), isSelected))
                {
                    m_studioConfig.PT.DebugInfoOutputMode = static_cast<DebugInfoOutput>(i);
                    m_pathTracerDirty = true;
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }
        ImGui::Unindent(IM_GUI_INDENTATION);
    }

    if (ImGui::Checkbox("Force Specular", &m_studioConfig.PT.DebugForceSpecular))
    {
        m_studioConfig.PT.DebugForceDiffuse = false;
        m_shaderDirty = true;
    }
    if (ImGui::Checkbox("Force Diffuse", &m_studioConfig.PT.DebugForceDiffuse))
    {
        m_studioConfig.PT.DebugForceSpecular = false;
        m_shaderDirty = true;
    }

    if (ImGui::Checkbox("Furnace Test (HDR)", &m_studioConfig.PT.FurnaceTestHdReflect))
    {
        m_studioConfig.PT.FurnaceTestHhEmit = false;
        m_shaderDirty = true;
    }
    if (ImGui::Checkbox("Furnace Test (HHE)", &m_studioConfig.PT.FurnaceTestHhEmit))
    {
        m_studioConfig.PT.FurnaceTestHdReflect = false;
        m_shaderDirty = true;
    }
#endif
}

void SceneStudio::guiRaster()
{
    ImGui::Spacing();
    ImGui::SeparatorText("Raster Settings##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    static const std::vector<const char*> c_rasterDebugModes = {
        "Position",
        "Normals (Vertex)",
        "Normals (Bumped)",
        "Tangents",
        "Binormals",
        "UV",
        "Directional Lighting",
        "Textures",
        "Texture Lighting",
        "Roughness",
        "Metalness",
        "Emission",
        "View Direction",
        "Half Vector",
        "Normal Distribution",
        "Fresnel Term",
        "Geometric Masking",
        "Reflection",
        "IBL Irradiance",
        "Microfacet Specular",
        "Microfacet Diffuse",
        "Microfacet Indirect Specular",
        "Microfacet D+S",
        "Microfacet D+S+I",
    };

    const char* curSelection = c_rasterDebugModes.at(m_studioConfig.Raster.Mode);
    if (ImGuiUtils::BeginComboWithTooltip("Debug Mode##xx", curSelection))
    {
        for (size_t i = 0; i < c_rasterDebugModes.size(); i++)
        {
            const bool isSelected = m_studioConfig.Raster.Mode == i;
            if (ImGui::Selectable(c_rasterDebugModes.at(i), isSelected))
            {
                m_studioConfig.Raster.Mode = static_cast<RasterDebugMode>(i);
            }

            if (isSelected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }
}

void SceneStudio::guiMain()
{
    ImGui::Spacing();
    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Scene##xx");
    ImGui::Indent(IM_GUI_INDENTATION);
    {
        const char* curName = m_sceneConfigs.at(m_currentScene).Name.c_str();
        if (ImGuiUtils::BeginComboWithTooltip("Scene##xx", curName))
        {
            for (size_t i = 0; i < m_sceneConfigs.size(); i++)
            {
                const bool isSelected = m_currentScene == i;
                if (ImGui::Selectable(m_sceneConfigs.at(i).Name.c_str(), isSelected))
                {
                    m_currentScene = i;
                    m_sceneDirty = true;
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        m_sceneDirty |= ImGui::Button("Reload Scene##xx");
    }

    ImGui::Spacing();
    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Render Backend##xx");
    ImGui::Indent(IM_GUI_INDENTATION);
    {
        static int e = m_studioConfig.Backend;
        int c = 0;
        m_envMapDirty |= ImGui::RadioButton("Forward", &e, c++); ImGui::SameLine();
        m_envMapDirty |= ImGui::RadioButton("Deferred", &e, c++); ImGui::SameLine();
        ImGui::RadioButton("Path Tracer", &e, c++);
        m_studioConfig.Backend = static_cast<RenderBackend>(e);
    }

    ImGui::Spacing();
    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Camera##xx");
    ImGui::Indent(IM_GUI_INDENTATION);
    {
        XMFLOAT3 pos = m_camera.GetCamera().GetPosition();
        m_pathTracerDirty |= ImGuiUtils::FwInputFloat3("Camera Position##xx", reinterpret_cast<float*>(&pos));
        m_camera.GetCamera().SetPosition(pos);

        float pitch = m_camera.GetCamera().GetPitch();
        float yaw = m_camera.GetCamera().GetYaw();
        m_pathTracerDirty |= ImGuiUtils::FwDragFloat("Camera Pitch##xx", &pitch, 0.01f);
        m_pathTracerDirty |= ImGuiUtils::FwDragFloat("Camera Yaw##xx", &yaw, 0.01f);
        m_camera.GetCamera().SetPitchYaw(pitch, yaw);

        if (ImGui::Button("Reset Camera to Scene Start##xx"))
        {
            ResetCameraToSceneStart();
            m_pathTracerDirty = true;
        }
        if (ImGui::Button("Reset Camera to Origin##xx"))
        {
            m_camera.GetCamera().SetPosition({0, 0, 0});
            m_camera.GetCamera().SetPitchYaw(0, 0);
            m_pathTracerDirty = true;
        }
    }

    ImGui::Spacing();
    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Directional Light##xx");
    ImGui::Indent(IM_GUI_INDENTATION);
    {
        bool changedDirLight = false;
        changedDirLight |= ImGuiUtils::FwInputFloat3("Direction##xx", reinterpret_cast<float*>(&m_studioConfig.DirLightDirection));
        changedDirLight |= ImGuiUtils::FwColorEdit3("Colour##xx", reinterpret_cast<float*>(&m_studioConfig.DirLightColor));
        changedDirLight |= ImGuiUtils::FwInputFloat("Intensity##xx", &m_studioConfig.DirLightIntensity);
        m_pathTracerDirty |= changedDirLight;
#ifdef _DEBUG
        m_debugLinesDirty |= changedDirLight;
#endif
    }

#ifdef _DEBUG
    ImGui::Spacing();
    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Debug Lines##xx");
    ImGui::Indent(IM_GUI_INDENTATION);
    m_pathTracerDirty |= ImGui::Checkbox("Enabled##xxxx", &m_studioConfig.DebugLinesEnabled);
    if (m_studioConfig.DebugLinesEnabled)
    {
        m_pathTracerDirty |= ImGui::Checkbox("Dir Light##xxxx", &m_studioConfig.DirLightDebugLineEnabled);
    }
#endif

    ImGui::Spacing();
    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Environment Map##xx");
    ImGui::Indent(IM_GUI_INDENTATION);
    {
        m_shaderDirty |= ImGui::Checkbox("Enabled##xx", &m_studioConfig.EnvMapEnabled);
        if (m_studioConfig.EnvMapEnabled)
        {
            static float cachedRotation = m_studioConfig.EnvMapRotation;
            ImGuiUtils::FwInputFloat("Rotation##xx", &cachedRotation);
            if (ImGui::Button("Apply Rotation"))
            {
                m_studioConfig.EnvMapRotation = cachedRotation;
                m_shaderDirty = true;
            }

            if (m_studioConfig.Backend == RenderBackend::ePathTracer)
            {
                const bool envMapEAChanged = ImGui::Checkbox("Equal-Area Map##xx", &m_studioConfig.PT.EnvMapIsEqualArea);
                m_sceneDirty |= envMapEAChanged;
                m_shaderDirty |= envMapEAChanged;
            }

            const auto currMap = wstringToString(m_envMapList.at(m_selectedEnvMapIdx));
            if (ImGuiUtils::BeginComboWithTooltip("Map##xx", currMap.c_str()))
            {
                for (size_t i = 0; i < m_envMapList.size(); i++)
                {
                    const bool isSelected = m_selectedEnvMapIdx == i;
                    if (ImGui::Selectable(wstringToString(m_envMapList.at(i)).c_str(), isSelected))
                    {
                        m_selectedEnvMapIdx = i;
                        m_envMapDirty = true;
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }
        }

        m_shaderDirty |= m_envMapDirty;
    }

    ImGui::Spacing();
    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Denoising##xx");
    ImGui::Indent(IM_GUI_INDENTATION);
    {
        static const std::vector<const char*> denoisingTypeMap = {
            "Box", "Gaussian", "Median(3x3)", "A-Trous", "NVIDIA NRD"
        };

        ImGui::Checkbox("Enabled##xxx", &m_studioConfig.Denoising.Enabled);
        if (m_studioConfig.Denoising.Enabled)
        {
            const auto currType = denoisingTypeMap.at(m_studioConfig.Denoising.Type);
            if (ImGuiUtils::BeginComboWithTooltip("Type##xx", currType))
            {
                for (size_t i = 0; i < denoisingTypeMap.size(); i++)
                {
                    const bool isSelected = m_studioConfig.Denoising.Type == i;
                    if (ImGui::Selectable(denoisingTypeMap.at(i), isSelected))
                    {
                        m_studioConfig.Denoising.Type = static_cast<DenoisingType>(i);
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            if (m_studioConfig.Denoising.Type == eBox || m_studioConfig.Denoising.Type == eGaussian)
            {
                ImGuiUtils::FwInputInt("Radius##xx", &m_studioConfig.Denoising.BoxRadius);
                m_studioConfig.Denoising.BoxRadius = std::max(0, m_studioConfig.Denoising.BoxRadius);
            }

            if (m_studioConfig.Denoising.Type == eATrous)
            {
                ImGui::InputInt("Iterations##xx", &m_studioConfig.Denoising.ATrousIterations);
                if (m_studioConfig.Denoising.ATrousIterations > MAX_ATROUS_ITERATIONS)
                    m_studioConfig.Denoising.ATrousIterations = MAX_ATROUS_ITERATIONS;
                ImGuiUtils::FwInputFloat("Color Phi##xx", &m_studioConfig.Denoising.ATrousPhiC);
                ImGuiUtils::FwInputFloat("Normals Phi##xx", &m_studioConfig.Denoising.ATrousPhiN);
                ImGuiUtils::FwInputFloat("Position Phi##xx", &m_studioConfig.Denoising.ATrousPhiP);
            }
        }
    }

    ImGui::Unindent(IM_GUI_INDENTATION);

#ifdef _DEBUG
    ImGui::Spacing();
    ImGui::SeparatorText("RMSE Debug Tool");
    {
        ImGui::Indent(IM_GUI_INDENTATION);

        ImGui::Text("%s", "Selected Slot:");
        static int e2 = m_rmseTesterSlot;
        int idx = 0;
        ImGui::Indent(IM_GUI_INDENTATION);
        ImGui::RadioButton(m_rmseTester.SlotAFilled() ? "Slot A (Filled)" : "Slot A", &e2, idx++);
        ImGui::RadioButton(m_rmseTester.SlotBFilled() ? "Slot B (Filled)" : "Slot B", &e2, idx++);
        ImGui::Unindent(IM_GUI_INDENTATION);
        m_rmseTesterSlot = static_cast<uint32_t>(e2);

        if (ImGui::Button("Take Snapshot"))
        {
            m_rmseTester.PrepareTakeSnapshot();
        }

        if (ImGui::Button("Compute RMSE"))
        {
            m_rmseTester.PrepareComputeRMSE();
        }
        ImGui::Text("%s", (std::string("RMSE: ") + std::to_string(m_rmseTester.GetComputedRMSE())).c_str());

        ImGui::Spacing();
        ImGui::Separator();

        static uint32_t goldenMaxFrames = 50;
        ImGuiUtils::FwInputUInt("Golden Frames##xxx", &goldenMaxFrames);
        static char path[256];
        ImGui::InputText("Golden File Path", path, 256);

        if (ImGui::Button("Compute Golden"))
        {
            m_rmseTester.BeginComputeGolden(goldenMaxFrames, path);
            m_ptContext.Reset();
            ResetCameraToSceneStart();
        }

        if (m_rmseTester.IsRunningGolden())
        {
            ImGui::SameLine(); ImGui::Text("Progress: %.2f/100%%", 100.0f * m_ptContext.GetFrameNum() / static_cast<float>(goldenMaxFrames));
        }

        if (ImGui::Button("Load Golden Image"))
        {
            m_rmseTester.PrepareLoadGolden(path);
        }

        ImGui::Spacing();
        ImGui::Separator();

        static uint32_t convergenceMaxFrames = 50;
        ImGuiUtils::FwInputUInt("Max Frames##xxx", &convergenceMaxFrames);
        static uint32_t frameInc = 1;
        ImGuiUtils::FwInputUInt("Frame Inc##xxx", &frameInc);
        static char testName[256];
        ImGui::InputText("Test Name", testName, 256);
        static bool plotAndShow = false;
        ImGui::Checkbox("Plot and Show", &plotAndShow);

        if (ImGui::Button("Convergence Test"))
        {
            m_rmseTester.BeginConvergenceTest(convergenceMaxFrames, testName, frameInc, plotAndShow);
            m_ptContext.Reset();
            ResetCameraToSceneStart();
        }

        if (m_rmseTester.IsRunningConvergence())
        {
            ImGui::SameLine(); ImGui::Text("Progress: %.2f%%", m_rmseTester.GetConvergenceTestPercent() * 100.0f);
        }

        ImGui::Spacing();
        ImGui::Separator();

        static std::vector<std::string> testNames = { "", "" };
        for (int i = 0; i < testNames.size(); i++)
        {
            char buff[256] = {};
            memcpy(buff, testNames[i].data(), testNames[i].size());
            std::string label = std::string("Test Name ") + std::to_string(i);
            ImGui::InputText(label.c_str(), buff, 256);
            testNames[i] = buff;
        }

        const int lastIdx = testNames.size() - 1;
        if (lastIdx > 1 && testNames[lastIdx].empty() && testNames[lastIdx - 1].empty())
            testNames.erase(testNames.end() - 1);
        else if (!testNames[lastIdx].empty())
            testNames.emplace_back("");

        if (ImGui::Button("Compare Tests"))
        {
            testNames.erase(testNames.end() - 1);
            m_rmseTester.CompareTests(testNames);
        }

        ImGui::Unindent(IM_GUI_INDENTATION);
    }
#endif
}

void SceneStudio::guiScene()
{
    const int sceneIdx = m_sceneConfigs.at(m_currentScene).SceneIdx;
    auto& objects = m_scenes.at(sceneIdx)->GetObjects();

    ImGui::Spacing();
    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Objects##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    for (int i = 0; i < objects.size(); i++)
    {
        const std::string name = std::string(objects[i]->GetName()) + " (" + objects[i]->GetMaterialForward()->GetName() + ")";
        const std::string tempID = "##" + std::to_string(i);
        if (ImGui::TreeNode((name + tempID).c_str()))
        {
            ImGui::Indent(IM_GUI_INDENTATION);

            const auto transform = objects[i]->GetTransform();

            XMFLOAT3 pos = transform->GetPosition();
            ImGuiUtils::FwInputFloat3(("Position" + tempID).c_str(), reinterpret_cast<float*>(&pos));
            transform->SetPosition(pos);

            XMFLOAT3 rot = transform->GetRotationE();
            ImGuiUtils::FwInputFloat3(("Rotation" + tempID).c_str(), reinterpret_cast<float*>(&rot));
            transform->SetRotationE(rot);

            XMFLOAT3 scale = transform->GetScale();
            ImGuiUtils::FwInputFloat3(("Scale" + tempID).c_str(), reinterpret_cast<float*>(&scale));
            transform->SetScale(scale);

            const XMFLOAT3 centroid = objects[i]->GetModel()->GetCentroid();
            ImGui::Text(("Centroid: (%f, %f, %f)" + tempID).c_str(), centroid.x, centroid.y, centroid.z);

            ImGui::Spacing();

            ImGui::Text(("Vertex Count: %lld" + tempID).c_str(), objects[i]->GetModel()->GetVertexCount());
            ImGui::Text(("Index Count: %lld" + tempID).c_str(), objects[i]->GetModel()->GetIndexCount());

            ImGui::Spacing();
            ImGui::SeparatorText(("Material" + tempID).c_str());

            Material* mat = objects[i]->GetMaterialForward();
            MaterialData* matData = mat->GetData();

            m_sceneDirty |= ImGuiUtils::FwColorEdit3(("Base Color Factor##xx" + tempID).c_str(), std::bit_cast<float*>(&matData->BaseColorFactor));
            m_sceneDirty |= ImGuiUtils::FwColorEdit3(("Emissive Color Factor##xx" + tempID).c_str(), std::bit_cast<float*>(&matData->EmissiveColor));

            m_sceneDirty |= ImGuiUtils::FwInputFloat(("Emissive Strength##xx" + tempID).c_str(), &matData->EmissiveStrength);
            m_sceneDirty |= ImGuiUtils::FwInputFloat(("Diffuse Probability##xx" + tempID).c_str(), &matData->DiffuseProbability);
            m_sceneDirty |= ImGuiUtils::FwInputFloat(("Roughness##xx" + tempID).c_str(), &matData->Roughness);
            m_sceneDirty |= ImGuiUtils::FwInputFloat(("Metalness##xx" + tempID).c_str(), &matData->Metalness);
            m_sceneDirty |= ImGuiUtils::FwInputFloat(("IoR##xx" + tempID).c_str(), &matData->IoR);

            m_sceneDirty |= ImGuiUtils::FwInputInt(("Diffuse Tex Idx##xx" + tempID).c_str(), &matData->BindlessTexDiffuse);
            m_sceneDirty |= ImGuiUtils::FwInputInt(("Normal Tex Idx##xx" + tempID).c_str(), &matData->BindlessTexNormal);

            bool isGlass = matData->Flags & PtMaterialFlags::eIsGlass;
            m_sceneDirty |= ImGui::Checkbox(("Is Glass##xx" + tempID).c_str(), &isGlass);
            matData->Flags = isGlass ? PtMaterialFlags::eIsGlass : PtMaterialFlags::eNone;

            ImGui::Unindent(IM_GUI_INDENTATION);
            ImGui::TreePop();
        }
    }

    if (m_sceneDirty)
    {
        m_ptContext.Reset();
#ifdef _DEBUG
        m_readbackManager.ClearReadbackData();
#endif
    }
}

void SceneStudio::guiHeapDebug()
{
    const auto& descriptorNameList = m_heap.GetDebugDescriptorList();
    const auto& descriptorNameListBindless = m_heap.GetDebugDescriptorListBindless();

    if (ImGui::CollapsingHeader("Binded Descriptors"))
    {
        ImGui::Indent(IM_GUI_INDENTATION);
        for (int i = 0; i < m_heap.GetCurrBindedDescriptorCount(); i++)
        {
            if (descriptorNameList.at(i))
                ImGui::Text("[%i] %s", i, descriptorNameList[i]);
            else
                ImGui::Text("[%i] Unnamed Descriptor", i);
        }
        ImGui::Unindent(IM_GUI_INDENTATION);;
    }

    if (ImGui::CollapsingHeader("Bindless Descriptors"))
    {
        ImGui::Indent(IM_GUI_INDENTATION);
        for (int i = 0; i < m_heap.GetCurrBindlessDescriptorCount(); i++)
        {
            if (descriptorNameListBindless.at(i))
                ImGui::Text("[%i] %s", i, descriptorNameListBindless[i]);
            else
                ImGui::Text("[%i] Unnamed Descriptor", i);
        }
        ImGui::Unindent(IM_GUI_INDENTATION);;
    }
}