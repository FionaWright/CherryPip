//
// Created by fionaw on 09/11/2025.
//

#include "System/pch.h"
#include <bit>

#include "Helper.h"
#include "ThirdParty/imgui/imgui.h"
#include "Apps/SceneStudio/Headers/SceneStudio.h"
#include "Render/Scene.h"
#include "System/Gui.h"

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

        if (ImGui::BeginTabItem("Scene"))
        {
            guiScene();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    Gui::EndWindow();
}

void SceneStudio::GuiPathTracer(const bool resetPT)
{
    ImGui::SeparatorText("Path-Tracer Stats##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    ImGui::Text("%s%i", "Frame Index: ", m_ptContext.GetFrameNum());
    ImGui::Text("%s%i", "Total SPP: ", m_ptContext.GetFrameNum() * m_studioConfig.PT.SPP);

    bool ptNeedsReset = resetPT;

    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Path-Tracer Settings##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    ptNeedsReset |= ImGui::Checkbox("Accumulation Enabled##xx", &m_studioConfig.PT.AccumulationEnabled);
    ptNeedsReset |= ImGui::Checkbox("Jitter Enabled##xx", &m_studioConfig.PT.JitterEnabled);
    m_shaderDirty |= ImGui::Checkbox("Russian Roulette##xx", &m_studioConfig.PT.RussianRouletteEnabled);

    if (m_studioConfig.PT.RussianRouletteEnabled)
    {
        int rrmb = static_cast<int>(m_studioConfig.PT.RussianRouletteMinBounces);
        ptNeedsReset |= ImGui::InputInt("Russian Roulette Min Bounces##xx", &rrmb);
        m_studioConfig.PT.RussianRouletteMinBounces = static_cast<uint32_t>(rrmb);
    }

    int spp = static_cast<int>(m_studioConfig.PT.SPP);
    ptNeedsReset |= ImGui::DragInt("SPP##xx", &spp, 1, 1, 256);
    m_studioConfig.PT.SPP = static_cast<uint32_t>(spp);

    int bounces = static_cast<int>(m_studioConfig.PT.NumBounces);
    ptNeedsReset |= ImGui::DragInt("Ray Bounces##xx", &bounces, 1, 0, 256);
    m_studioConfig.PT.NumBounces = static_cast<uint32_t>(bounces);

    int maxFrame = static_cast<int>(m_studioConfig.PT.MaxFrameNum);
    ptNeedsReset |= ImGui::InputInt("Max Frames##xx", &maxFrame);
    m_studioConfig.PT.MaxFrameNum = static_cast<uint32_t>(maxFrame);

    m_shaderDirty |= ImGui::Checkbox("Dir Light Enabled##xx", &m_studioConfig.PT.DirLightEnabled);
    ptNeedsReset |= ImGui::InputFloat("Dir Light Radius (R)##xx", &m_studioConfig.PT.DirLightCosAngularRadius);

    m_shaderDirty |= ImGui::Checkbox("Normal Maps Enabled##xx", &m_studioConfig.PT.NormalMapsEnabled);

    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Path-Tracer Tools##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    ptNeedsReset |= ImGui::Button("Reset PathTracer##xx");

#ifdef _DEBUG
    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Path-Tracer Debug##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    const bool prevReadbackEnabled = m_studioConfig.PT.ReadbackEnabled;
    ImGui::Checkbox("Enable Readback##xx", &m_studioConfig.PT.ReadbackEnabled);

    if (m_studioConfig.PT.ReadbackEnabled)
    {
        m_readbackManager.GUI(prevReadbackEnabled, m_studioConfig.PT.ReadbackEveryFrame);
    }

    static const std::vector<const char*> c_debugBufferStrMap = {
        "Normals (Shaded)",
        "Normals (Geometric)",
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
        "Roughness",
        "Metalness",
    };

    static int e = m_studioConfig.PT.Mode;
    ImGui::Text("%s", "Path Tracer Mode:");
    ImGui::Indent(IM_GUI_INDENTATION);
    int idx = 0;
    m_shaderDirty |= ImGui::RadioButton("Lambert", &e, idx++);
    m_shaderDirty |= ImGui::RadioButton("Glossy", &e, idx++);
    m_shaderDirty |= ImGui::RadioButton("Glass", &e, idx++);
    m_shaderDirty |= ImGui::RadioButton("GGX/Smith-Schlick Microfacet", &e, idx++);
    m_shaderDirty |= ImGui::RadioButton("Furnace Test (HDR)", &e, idx++);
    m_shaderDirty |= ImGui::RadioButton("Furnace Test (HHE)", &e, idx++);
    ImGui::Unindent(IM_GUI_INDENTATION);
    m_studioConfig.PT.Mode = static_cast<PathTracerMode>(e);

    m_shaderDirty |= ImGui::Checkbox("Debug Buffers##xx", &m_studioConfig.PT.DebugMode);
    if (m_studioConfig.PT.DebugMode)
    {
        ImGui::Indent(IM_GUI_INDENTATION);
        const char* curSelection = c_debugBufferStrMap.at(static_cast<uint32_t>(m_studioConfig.PT.DebugBufferIdx));
        if (ImGui::BeginCombo("Debug Buffer##xx", curSelection))
        {
            for (size_t i = 0; i < c_debugBufferStrMap.size(); i++)
            {
                const bool isSelected = static_cast<uint32_t>(m_studioConfig.PT.DebugBufferIdx) == i;
                if (ImGui::Selectable(c_debugBufferStrMap.at(i), isSelected))
                {
                    m_studioConfig.PT.DebugBufferIdx = static_cast<DebugBuffer>(i);
                    ptNeedsReset = true;
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::Unindent(IM_GUI_INDENTATION);
            ImGui::EndCombo();
        }
    }

    ptNeedsReset |= m_shaderDirty;

    if (ptNeedsReset && m_studioConfig.PT.ReadbackEveryFrame)
        m_readbackManager.SetInReadbackProcess(true);
    else if (!m_studioConfig.PT.ReadbackEveryFrame)
        m_readbackManager.SetInReadbackProcess(false);
#endif

    ImGui::Unindent(IM_GUI_INDENTATION);
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
        "Microfacet Specular",
        "Microfacet Diffuse",
        "Microfacet Final",
    };

    const char* curSelection = c_rasterDebugModes.at(m_studioConfig.Raster.Mode);
    if (ImGui::BeginCombo("Debug Mode##xx", curSelection))
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
    bool resetPT = false;

    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Scene##xx");
    ImGui::Indent(IM_GUI_INDENTATION);
    {
        const char* curName = m_sceneConfigs.at(m_currentScene).Name.c_str();
        if (ImGui::BeginCombo("Scene##xx", curName))
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

    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Render Backend##xx");
    ImGui::Indent(IM_GUI_INDENTATION);
    {
        static int e = m_studioConfig.Backend;
        ImGui::RadioButton("Forward", &e, 0); ImGui::SameLine();
        ImGui::RadioButton("Path Tracer", &e, 1);
        m_studioConfig.Backend = static_cast<RenderBackend>(e);
    }

    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Camera##xx");
    ImGui::Indent(IM_GUI_INDENTATION);
    {
        XMFLOAT3 pos = m_camera.GetCamera().GetPosition();
        resetPT |= ImGui::InputFloat3("Camera Position##xx", reinterpret_cast<float*>(&pos));
        m_camera.GetCamera().SetPosition(pos);

        float pitch = m_camera.GetCamera().GetPitch();
        float yaw = m_camera.GetCamera().GetYaw();
        resetPT |= ImGui::DragFloat("Camera Pitch##xx", &pitch, 0.01f);
        resetPT |= ImGui::DragFloat("Camera Yaw##xx", &yaw, 0.01f);
        m_camera.GetCamera().SetPitchYaw(pitch, yaw);

        if (ImGui::Button("Reset Camera to Scene Start##xx"))
        {
            const auto currScene = m_sceneConfigs.at(m_currentScene);
            m_camera.GetCamera().SetPosition(currScene.InitialCamPos);
            m_camera.GetCamera().SetPitchYaw(currScene.InitialCamPitchYaw.x, currScene.InitialCamPitchYaw.y);
            resetPT = true;
        }
        if (ImGui::Button("Reset Camera to Origin##xx"))
        {
            m_camera.GetCamera().SetPosition({0, 0, 0});
            m_camera.GetCamera().SetPitchYaw(0, 0);
            resetPT = true;
        }
    }

    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Directional Light##xx");
    ImGui::Indent(IM_GUI_INDENTATION);
    {
        resetPT |= ImGui::InputFloat3("Direction##xx", reinterpret_cast<float*>(&m_studioConfig.DirLightDirection));
        resetPT |= ImGui::ColorEdit3("Colour##xx", reinterpret_cast<float*>(&m_studioConfig.DirLightColor));
        resetPT |= ImGui::InputFloat("Intensity##xx", &m_studioConfig.DirLightIntensity);
    }

    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Environment Map##xx");
    ImGui::Indent(IM_GUI_INDENTATION);
    {
        m_shaderDirty |= ImGui::Checkbox("Enabled##xx", &m_studioConfig.EnvMapEnabled);
        if (m_studioConfig.EnvMapEnabled)
        {
            m_envMapDirty |= ImGui::InputFloat("Rotation##xx", &m_studioConfig.EnvMapRotation);

            if (m_studioConfig.Backend == RenderBackend::ePathTracer)
            {
                const bool envMapEAChanged = ImGui::Checkbox("Equal-Area Map##xx", &m_studioConfig.PT.EnvMapIsEqualArea);
                m_sceneDirty |= envMapEAChanged;
                m_shaderDirty |= envMapEAChanged;
            }

            const auto currMap = wstringToString(m_envMapList.at(m_selectedEnvMapIdx));
            if (ImGui::BeginCombo("Map##xx", currMap.c_str()))
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
            if (ImGui::BeginCombo("Type##xx", currType))
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
                ImGui::InputInt("Radius##xx", &m_studioConfig.Denoising.BoxRadius);
                m_studioConfig.Denoising.BoxRadius = std::max(0, m_studioConfig.Denoising.BoxRadius);
            }

            if (m_studioConfig.Denoising.Type == eATrous)
            {
                ImGui::InputInt("Iterations##xx", &m_studioConfig.Denoising.ATrousIterations);
                if (m_studioConfig.Denoising.ATrousIterations > MAX_ATROUS_ITERATIONS)
                    m_studioConfig.Denoising.ATrousIterations = MAX_ATROUS_ITERATIONS;
                ImGui::InputFloat("Color Phi##xx", &m_studioConfig.Denoising.ATrousPhiC);
                ImGui::InputFloat("Normals Phi##xx", &m_studioConfig.Denoising.ATrousPhiN);
                ImGui::InputFloat("Position Phi##xx", &m_studioConfig.Denoising.ATrousPhiP);
            }
        }
    }

    ImGui::Unindent(IM_GUI_INDENTATION);
    switch (m_studioConfig.Backend)
    {
    case eForward:
        GuiRaster();
        break;
    case ePathTracer:
        resetPT |= m_sceneDirty;
        GuiPathTracer(resetPT);
        break;
    default:
        break;
    }
}

void SceneStudio::guiScene()
{
    const int sceneIdx = m_sceneConfigs.at(m_currentScene).SceneIdx;
    auto& objects = m_scenes.at(sceneIdx)->GetObjects();

    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Objects##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    for (int i = 0; i < objects.size(); i++)
    {
        const std::string name = std::string(objects[i]->GetName()) + " (" + objects[i]->GetMaterial()->GetName() + ")";
        if (ImGui::TreeNode(name.c_str()))
        {
            ImGui::Indent(IM_GUI_INDENTATION);

            const auto transform = objects[i]->GetTransform();

            XMFLOAT3 pos = transform->GetPosition();
            ImGui::InputFloat3("Position##xx", reinterpret_cast<float*>(&pos));
            transform->SetPosition(pos);

            XMFLOAT3 rot = transform->GetRotationE();
            ImGui::InputFloat3("Rotation##xx", reinterpret_cast<float*>(&rot));
            transform->SetRotationE(rot);

            XMFLOAT3 scale = transform->GetScale();
            ImGui::InputFloat3("Scale##xx", reinterpret_cast<float*>(&scale));
            transform->SetScale(scale);

            const XMFLOAT3 centroid = objects[i]->GetModel()->GetCentroid();
            ImGui::Text("Centroid: (%f, %f, %f)", centroid.x, centroid.y, centroid.z);

            ImGui::Spacing();

            ImGui::Text("Vertex Count: %lld", objects[i]->GetModel()->GetVertexCount());
            ImGui::Text("Index Count: %lld", objects[i]->GetModel()->GetIndexCount());

            ImGui::Spacing();
            ImGui::SeparatorText("Material##xx");

            Material* mat = objects[i]->GetMaterial();
            MaterialData* matData = mat->GetData();

            m_sceneDirty |= ImGui::ColorEdit3("Base Color Factor##xx", std::bit_cast<float*>(&matData->BaseColorFactor));

            m_sceneDirty |= ImGui::InputFloat("Emissive Strength##xx", &matData->EmissiveStrength);
            m_sceneDirty |= ImGui::InputFloat("Diffuse Probability##xx", &matData->DiffuseProbability);
            m_sceneDirty |= ImGui::InputFloat("Roughness##xx", &matData->Roughness);
            m_sceneDirty |= ImGui::InputFloat("Metalness##xx", &matData->Metalness);
            m_sceneDirty |= ImGui::InputFloat("IoR##xx", &matData->IoR);

            m_sceneDirty |= ImGui::InputInt("Diffuse Tex Idx##xx", &matData->BindlessTexDiffuse);
            m_sceneDirty |= ImGui::InputInt("Normal Tex Idx##xx", &matData->BindlessTexNormal);

            bool isGlass = matData->Flags & PtMaterialFlags::eIsGlass;
            m_sceneDirty |= ImGui::Checkbox("Is Glass##xx", &isGlass);
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