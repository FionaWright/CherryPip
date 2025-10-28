//
// Created by fionaw on 26/10/2025.
//

#ifndef PT_HOTRELOADER_H
#define PT_HOTRELOADER_H
#include <string>
#include <unordered_map>

#include "HWI/Shader.h"

class HotReloader
{
public:
    static void AssignShaderVsPs(const std::wstring& vs, const std::wstring& ps, Shader* shader, const D3D12_INPUT_LAYOUT_DESC& ild, ID3D12RootSignature* rootSig);
    static void AssignShaderCs(const std::wstring& cs, Shader* shader, ID3D12RootSignature* rootSig);
    static void CheckFiles(D3D* d3d);
    static void PendFullReload() { m_pendingFullReload = true; }

private:
    struct HotInfoVsPs
    {
        std::wstring VS, VSPath;
        std::wstring PS, PSPath;

        Shader* ShaderPtr;
        std::vector<D3D12_INPUT_ELEMENT_DESC> ild;

        ID3D12RootSignature* RootSig;

        std::time_t TimeStampVS;
        std::time_t TimeStampPS;
    };

    struct HotInfoCs
    {
        std::wstring CS, CSPath;

        Shader* ShaderPtr;
        ID3D12RootSignature* RootSig;

        std::time_t TimeStampCS;
    };

    static std::vector<HotInfoVsPs> s_shadersVsPs;
    static std::vector<HotInfoCs> s_shadersCs;

    static bool m_pendingFullReload;
};


#endif //PT_HOTRELOADER_H