//
// Created by fiona on 24/09/2025.
//

#ifndef PT_FILEHELPER_H
#define PT_FILEHELPER_H

#include <string>
#include <windows.h>

class FileHelper
{
public:
    static void Init();
    static std::wstring GetAssetFullPath(LPCWSTR assetName);
    static std::wstring GetAssetShaderFullPath(LPCWSTR assetName);

private:
    static std::wstring m_assetsPath;
};


#endif //PT_FILEHELPER_H