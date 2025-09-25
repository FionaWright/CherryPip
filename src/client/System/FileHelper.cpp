//
// Created by fiona on 24/09/2025.
//

#include "System/FileHelper.h"

std::wstring FileHelper::m_assetsPath;

void FileHelper::Init()
{
    constexpr UINT c_pathSize = 256;
    WCHAR path[c_pathSize];

    DWORD size = GetModuleFileNameW(nullptr, path, c_pathSize);
    if (size == 0 || size == c_pathSize)
    {
        // Method failed or path was truncated.
        throw std::exception();
    }

    WCHAR* lastSlash = wcsrchr(path, L'\\');
    if (lastSlash)
    {
        *(lastSlash + 1) = L'\0';
    }

    m_assetsPath = std::wstring(path) + L"Assets/";
}

std::wstring FileHelper::GetAssetFullPath(LPCWSTR assetName)
{
    return m_assetsPath + assetName;
}

std::wstring FileHelper::GetAssetShaderFullPath(LPCWSTR assetName)
{
    return m_assetsPath + L"Shaders/" + assetName;
}