//
// Created by fiona on 24/09/2025.
//

#ifndef PT_FILEHELPER_H
#define PT_FILEHELPER_H

#include <string>
#include <vector>
#include <windows.h>

class FileHelper
{
public:
    static void Init();
    static std::wstring GetAssetsPath() { return m_assetsPath; }
    static std::wstring GetAssetFullPath(LPCWSTR assetName);
    static std::wstring GetAssetTextureFullPath(LPCWSTR assetName);
    static std::wstring GetAssetShaderFullPath(LPCWSTR assetName);
    static std::wstring GetAssetModelFullPath(LPCWSTR assetName);

    static std::vector<uint8_t> ReadFileToByteVector(const std::wstring& filename);
    static HRESULT ReadDataFromFile(LPCWSTR filename, byte** data, UINT* size);
    static HRESULT ReadDataFromDDSFile(LPCWSTR filename, byte** data, UINT* offset, UINT* size);

private:
    static std::wstring m_assetsPath;
};

#endif //PT_FILEHELPER_H
