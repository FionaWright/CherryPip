#include "System/pch.h"

#include "Apps/SceneStudio/Headers/PathTracer.h"
#include "System/ImGuiUtils.h"

#define IM_GUI_INDENTATION 20 // Temp

void PathTracer::RenderGUI(bool& outPtDirty, bool& outShaderDirty, bool& outSceneDirty, XMFLOAT2 mousePosOnClick, bool isSpectral)
{    
    ImGui::SeparatorText("Stats##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    ImGui::Text("%s%i", "Frame Index: ", m_ptContext.GetFrameNum());
    ImGui::Text("%s%i", "Total SPP: ", m_ptContext.GetFrameNum() * m_config.SPP);

    ImGui::Spacing();
    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Settings##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    if (isSpectral)
    {
        ImGui::Spacing();
        ImGui::SeparatorText("Spectral Settings##xx");
        ImGui::Indent(IM_GUI_INDENTATION);

        static int e = m_spectralConfig.SamplingMode;
        ImGui::Text("%s", "Sampling Mode:");
        int idx = 0;
        ImGui::Indent(IM_GUI_INDENTATION);
        outShaderDirty |= ImGui::RadioButton("Full Spectrum", &e, idx++);
        outShaderDirty |= ImGui::RadioButton("Single Wavelength", &e, idx++);
        outShaderDirty |= ImGui::RadioButton("Hero", &e, idx++);
        ImGui::Unindent(IM_GUI_INDENTATION);
        m_spectralConfig.SamplingMode = static_cast<SpectralSamplingMode>(e);

        if (m_spectralConfig.SamplingMode == eSingleWavelength)
        {
            outShaderDirty |= ImGui::Checkbox("Force Wavelength", &m_spectralConfig.DebugForceWavelengthEnabled);
            if (m_spectralConfig.DebugForceWavelengthEnabled)
                outShaderDirty |= ImGui::InputFloat("Wavelength", &m_spectralConfig.DebugForcedWavelength);
        }

        ImGui::Unindent(IM_GUI_INDENTATION);
        ImGui::Spacing();
        ImGui::Spacing();
    }

    outPtDirty |= ImGui::Checkbox("Accumulation Enabled##xx", &m_config.AccumulationEnabled);
    outShaderDirty |= ImGui::Checkbox("Jitter Enabled##xx", &m_config.JitterEnabled);
    outShaderDirty |= ImGui::Checkbox("Russian Roulette##xx", &m_config.RussianRouletteEnabled);

    if (m_config.RussianRouletteEnabled)
        outPtDirty |= ImGuiUtils::FwInputUInt("Russian Roulette Min Bounces##xx", &m_config.RussianRouletteMinBounces);

    int spp = static_cast<int>(m_config.SPP);
    outPtDirty |= ImGuiUtils::FwDragInt("SPP##xx", &spp, 1, 1, 256);
    m_config.SPP = static_cast<uint32_t>(spp);

    int bounces = static_cast<int>(m_config.NumBounces);
    outPtDirty |= ImGuiUtils::FwDragInt("Ray Bounces##xx", &bounces, 1, 0, 256);
    m_config.NumBounces = static_cast<uint32_t>(bounces);

    const uint32_t prevMaxFrames = m_config.MaxFrameNum;
    if (ImGuiUtils::FwInputUInt("Max Frames##xx", &m_config.MaxFrameNum))
        outPtDirty |= prevMaxFrames > m_config.MaxFrameNum || prevMaxFrames == 0;

    outShaderDirty |= ImGui::Checkbox("Dir Light Enabled##xx", &m_config.DirLightEnabled);
    outPtDirty |= ImGuiUtils::FwInputFloat("Dir Light Radius (R)##xx", &m_config.DirLightCosAngularRadius);
    outShaderDirty |= ImGui::Checkbox("Dir Light Is Distant", &m_config.DirLightIsDistant);

    const bool envMapEAChanged = ImGui::Checkbox("Equal-Area Map##xx", &m_config.EnvMapIsEqualArea);
    outSceneDirty |= envMapEAChanged;
    outShaderDirty |= envMapEAChanged;

    outShaderDirty |= ImGui::Checkbox("Normal Maps Enabled##xx", &m_config.NormalMapsEnabled);
    outShaderDirty |= ImGui::Checkbox("Alpha Testing Enabled##xx", &m_config.AlphaTestingEnabled);
    outShaderDirty |= ImGui::Checkbox("Gamma Correction Enabled##xx", &m_config.GammaCorrection);

    outShaderDirty |= ImGuiUtils::FwInputFloat("Firefly Threshold", &m_config.FireflyThreshold);

    outShaderDirty |= ImGui::Checkbox("Depth Of Field Enabled", &m_config.DepthOfFieldEnabled);
    if (m_config.DepthOfFieldEnabled)
    {
        ImGui::Indent(IM_GUI_INDENTATION);
        outShaderDirty |= ImGuiUtils::FwDragFloat("Focal Distance", &m_config.DofFocalDist, 0.03f);
        outShaderDirty |= ImGuiUtils::FwDragFloat("Lens Radius", &m_config.DofLensRadius, 0.01f);
        ImGui::Unindent(IM_GUI_INDENTATION);
    }

    static int e = m_config.LightingModel;
    ImGui::Text("%s", "Lighting Model:");
    int idx = 0;
    ImGui::Indent(IM_GUI_INDENTATION);
    outShaderDirty |= ImGui::RadioButton("Lambert", &e, idx++);
    outShaderDirty |= ImGui::RadioButton("Glossy", &e, idx++);
    outShaderDirty |= ImGui::RadioButton("Microfacet", &e, idx++);
    ImGui::Unindent(IM_GUI_INDENTATION);
    m_config.LightingModel = static_cast<PathTracerLightingModel>(e);

    ImGui::Text("%s", "RNG Sampling Strategy:");
    static int e3 = m_config.SamplingStrat;
    idx = 0;
    ImGui::Indent(IM_GUI_INDENTATION);
    outShaderDirty |= ImGui::RadioButton("Independent", &e3, idx++);
    outShaderDirty |= ImGui::RadioButton("Halton", &e3, idx++);
    outShaderDirty |= ImGui::RadioButton("Apple Halton", &e3, idx++);
    outShaderDirty |= ImGui::RadioButton("Owen Scrambled Halton", &e3, idx++);
    ImGui::Unindent(IM_GUI_INDENTATION);
    m_config.SamplingStrat = static_cast<PathTracerSamplingStrategy>(e3);

    if (m_config.LightingModel == eMicrofacet)
    {
        ImGui::Text("%s", "Microfacet Normal Distribution Function:");
        static int e2 = m_config.NdfType;
        idx = 0;
        ImGui::Indent(IM_GUI_INDENTATION);
        outShaderDirty |= ImGui::RadioButton("GGX", &e2, idx++);
        outShaderDirty |= ImGui::RadioButton("Beckmann", &e2, idx++);
        ImGui::Unindent(IM_GUI_INDENTATION);
        m_config.NdfType = static_cast<MicrofacetNdfType>(e2);

        ImGui::Text("%s", "Microfacet Masking Function:");
        static int e4 = m_config.MaskingType;
        idx = 0;
        ImGui::Indent(IM_GUI_INDENTATION);
        outShaderDirty |= ImGui::RadioButton("Smith", &e4, idx++);
        outShaderDirty |= ImGui::RadioButton("V-Cavity", &e4, idx++);
        ImGui::Unindent(IM_GUI_INDENTATION);
        m_config.MaskingType = static_cast<MicrofacetMaskingType>(e4);

        outShaderDirty |= ImGui::Checkbox("Anisotropy Enabled", &m_config.AnisotropyEnabled);
        outShaderDirty |= ImGui::Checkbox("Sample Visible Normals", &m_config.SampleVisibleNormals);
    }

    if (m_config.LightingModel != eLambertDiff)
        outShaderDirty |= ImGui::Checkbox("Glass Model Enabled##xx", &m_config.GlassModelEnabled);
    outShaderDirty |= ImGui::Checkbox("Importance Sampling Enabled##xx", &m_config.ImportanceSamplingEnabled);

    ImGui::Spacing();
    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Tools##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    outPtDirty |= ImGui::Button("Reset PathTracer##xx");

#ifdef _DEBUG
    ImGui::Spacing();
    ImGui::Unindent(IM_GUI_INDENTATION);
    ImGui::SeparatorText("Debug##xx");
    ImGui::Indent(IM_GUI_INDENTATION);

    const bool prevReadbackEnabled = m_config.ReadbackEnabled;
    ImGui::Checkbox("Enable Readback##xx", &m_config.ReadbackEnabled);

    if (m_config.ReadbackEnabled)
    {
        m_readbackManager.GUI(prevReadbackEnabled, m_config.ReadbackEveryFrame);
    }

    outShaderDirty |= ImGui::Checkbox("Path Visualization Enabled", &m_config.DebugPathVisualization);
    if (m_config.DebugPathVisualization)
    {
        ImGui::Indent(IM_GUI_INDENTATION);

        const bool validMousePos = mousePosOnClick.x != -1 && mousePosOnClick.y != -1;
        if (validMousePos)
        {
            m_takePathVisualizationSnapshot |= ImGui::Button("Take Snapshot##xxx");
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

    outShaderDirty |= ImGui::Checkbox("Info Output Enabled##xx", &m_config.DebugInfoOutputEnabled);
    if (m_config.DebugInfoOutputEnabled)
    {
        ImGui::Indent(IM_GUI_INDENTATION);
        const char* curSelection = c_debugBufferStrMap.at(static_cast<uint32_t>(m_config.DebugInfoOutputMode));
        if (ImGuiUtils::BeginComboWithTooltip("Info Output Mode##xx", curSelection))
        {
            for (size_t i = 0; i < c_debugBufferStrMap.size(); i++)
            {
                const bool isSelected = static_cast<uint32_t>(m_config.DebugInfoOutputMode) == i;
                if (ImGui::Selectable(c_debugBufferStrMap.at(i), isSelected))
                {
                    m_config.DebugInfoOutputMode = static_cast<DebugInfoOutput>(i);
                    outPtDirty = true;
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }
        ImGui::Unindent(IM_GUI_INDENTATION);
    }

    if (ImGui::Checkbox("Force Specular", &m_config.DebugForceSpecular))
    {
        m_config.DebugForceDiffuse = false;
        outShaderDirty = true;
    }
    if (ImGui::Checkbox("Force Diffuse", &m_config.DebugForceDiffuse))
    {
        m_config.DebugForceSpecular = false;
        outShaderDirty = true;
    }

    if (ImGui::Checkbox("Force Reflect", &m_config.DebugForceReflect))
    {
        m_config.DebugForceRefract = false;
        outShaderDirty = true;
    }
    if (ImGui::Checkbox("Force Refract", &m_config.DebugForceRefract))
    {
        m_config.DebugForceReflect = false;
        outShaderDirty = true;
    }

    if (ImGui::Checkbox("Furnace Test (HDR)", &m_config.FurnaceTestHdReflect))
    {
        m_config.FurnaceTestHhEmit = false;
        outShaderDirty = true;
    }
    if (ImGui::Checkbox("Furnace Test (HHE)", &m_config.FurnaceTestHhEmit))
    {
        m_config.FurnaceTestHdReflect = false;
        outShaderDirty = true;
    }
#endif
}