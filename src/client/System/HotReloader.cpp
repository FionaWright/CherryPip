//
// Created by fionaw on 26/10/2025.
//

#include "System/HotReloader.h"

#include <filesystem>
#include <iostream>

#include "System/FileHelper.h"

std::vector<HotReloader::HotInfoVsPs> HotReloader::s_shadersVsPs;
std::vector<HotReloader::HotInfoCs> HotReloader::s_shadersCs;
bool HotReloader::m_pendingFullReload = false;

std::time_t getTimestamp(const std::wstring& path)
{
    const auto ftime = std::filesystem::last_write_time(path);

    const auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - std::filesystem::file_time_type::clock::now()
        + std::chrono::system_clock::now()
    );

    return std::chrono::system_clock::to_time_t(sctp);
}

void HotReloader::AssignShaderVsPs(const std::wstring& vs, const std::wstring& ps, Shader* shader,
                                   const D3D12_INPUT_LAYOUT_DESC& ild, ID3D12RootSignature* rootSig)
{
    const std::wstring vsPath = FileHelper::GetAssetShaderFullPath(vs.c_str());
    const std::wstring psPath = FileHelper::GetAssetShaderFullPath(ps.c_str());

    if (!std::filesystem::exists(vsPath) || !std::filesystem::exists(psPath))
        return;

    std::vector<D3D12_INPUT_ELEMENT_DESC> ildElements;
    for (int i = 0; i < ild.NumElements; i++)
        ildElements.emplace_back(ild.pInputElementDescs[i]);

    const std::time_t timeStampVS = getTimestamp(vsPath);
    const std::time_t timeStampPS = getTimestamp(psPath);

    HotInfoVsPs info = {vs, vsPath, ps, psPath, shader, ildElements, rootSig, timeStampVS, timeStampPS};
    s_shadersVsPs.emplace_back(info);
}

void HotReloader::AssignShaderCs(const std::wstring& cs, Shader* shader,
                                 ID3D12RootSignature* rootSig)
{
    const std::wstring csPath = FileHelper::GetAssetShaderFullPath(cs.c_str());

    if (!std::filesystem::exists(csPath))
        return;

    const std::time_t timeStampCS = getTimestamp(csPath);

    HotInfoCs info = {cs, csPath, shader, rootSig, timeStampCS};
    s_shadersCs.emplace_back(info);
}

void HotReloader::CheckFiles(D3D* d3d)
{
#ifndef _DEBUG
    std::cout << "ERROR: Attempted to hot reload on release mode" << std::endl;
    return;
#endif

    for (int i = 0; i < s_shadersVsPs.size(); i++)
    {
        if (!s_shadersVsPs[i].ShaderPtr || !s_shadersVsPs[i].RootSig)
            continue;

        const auto& vsPath = s_shadersVsPs[i].VSPath;
        const auto& psPath = s_shadersVsPs[i].PSPath;

        if (!std::filesystem::exists(vsPath) || !std::filesystem::exists(psPath))
            continue;

        const std::time_t timeStampVS = getTimestamp(vsPath);
        const std::time_t timeStampPS = getTimestamp(psPath);

        if (!m_pendingFullReload && timeStampVS == s_shadersVsPs[i].TimeStampVS && timeStampPS == s_shadersVsPs[i].TimeStampPS)
            continue;

        std::cout << "Hot reloading shader" << std::endl;

        d3d->Flush();

        const D3D12_INPUT_LAYOUT_DESC ild = { s_shadersVsPs[i].ild.data(), static_cast<UINT>(s_shadersVsPs[i].ild.size()) };
        s_shadersVsPs[i].ShaderPtr->InitVsPs(s_shadersVsPs[i].VS.c_str(), s_shadersVsPs[i].PS.c_str(), ild, d3d->GetDevice(),
                                             s_shadersVsPs[i].RootSig);

        s_shadersVsPs[i].TimeStampVS = timeStampVS;
        s_shadersVsPs[i].TimeStampPS = timeStampPS;
    }

    for (int i = 0; i < s_shadersCs.size(); i++)
    {
        if (!s_shadersCs[i].ShaderPtr || !s_shadersCs[i].RootSig)
            continue;

        const auto& csPath = s_shadersCs[i].CSPath;

        if (!std::filesystem::exists(csPath))
            continue;

        const std::time_t timeStampCS = getTimestamp(csPath);

        if (!m_pendingFullReload && timeStampCS == s_shadersCs[i].TimeStampCS)
            continue;

        std::cout << "Hot reloading shader" << std::endl;

        d3d->Flush();

        s_shadersCs[i].ShaderPtr->InitCs(s_shadersCs[i].CS.c_str(), d3d->GetDevice(), s_shadersCs[i].RootSig);
        s_shadersCs[i].TimeStampCS = timeStampCS;
    }

    m_pendingFullReload = false;
}
