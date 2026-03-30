//
// Created by fionaw on 09/11/2025.
//

#include "System/pch.h"
#include <bit>

#include "Helper.h"
#include "imgui.h"
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
            m_pathTracer.RenderGUI(m_pathTracerDirty, m_shaderDirty, m_sceneDirty, m_mousePosOnClick, m_studioConfig.Backend == eSpectralTracer);
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
    m_pathTracerDirty |= m_sceneDirty;

#ifdef _DEBUG
    if (m_pathTracerDirty && m_pathTracer.GetConfig().ReadbackEveryFrame)
        m_pathTracer.GetReadbackManager()->SetInReadbackProcess(true);
    else if (!m_pathTracer.GetConfig().ReadbackEveryFrame)
        m_pathTracer.GetReadbackManager()->SetInReadbackProcess(false);
#endif

    if (m_pathTracerDirty)
    {
        m_pathTracer.Reset();
        m_pathTracerDirty = false;
    }

    Gui::EndWindow();
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
        m_envMapDirty |= ImGui::RadioButton("Forward", &e, c++);
        m_envMapDirty |= ImGui::RadioButton("Deferred", &e, c++);
        m_shaderDirty |= ImGui::RadioButton("Path Tracer", &e, c++);
        m_shaderDirty |= ImGui::RadioButton("Spectral Tracer", &e, c++);
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
        changedDirLight |= ImGuiUtils::FwInputFloat3("Direction##xx", reinterpret_cast<float*>(&m_studioConfig.DirLight.DirLightDirection));
        changedDirLight |= ImGuiUtils::FwColorEdit3("Colour##xx", reinterpret_cast<float*>(&m_studioConfig.DirLight.DirLightColor));
        changedDirLight |= ImGuiUtils::FwInputFloat("Intensity##xx", &m_studioConfig.DirLight.DirLightIntensity);
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
        m_pathTracerDirty |= ImGui::Checkbox("Dir Light##xxxx", &m_studioConfig.DirLight.DirLightDebugLineEnabled);
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
            m_pathTracer.Reset();
            ResetCameraToSceneStart();
        }

        if (m_rmseTester.IsRunningGolden())
        {
            ImGui::SameLine(); ImGui::Text("Progress: %.2f/100%%", 100.0f * m_pathTracer.GetContext().GetFrameNum() / static_cast<float>(goldenMaxFrames));
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
            m_pathTracer.Reset();
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

        static bool logPlot = false;
        ImGui::Checkbox("Logarithmic Y-Axis", &logPlot);

        if (ImGui::Button("Compare Tests"))
        {
            testNames.erase(testNames.end() - 1);
            m_rmseTester.CompareTests(testNames, logPlot);
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
        m_pathTracer.Reset();
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