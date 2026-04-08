//
// Created by fiona on 19/01/2026.
//


#include "System/ResourceSharer.h"

#include <ranges>

#include "Helper.h"

std::unordered_map<std::string, std::shared_ptr<Texture>> ResourceSharer::s_databaseTex;
std::unordered_map<std::wstring, uint32_t> ResourceSharer::s_databaseBindless;

std::string CanonicalizePath(const std::string& path) {
    std::string out;
    out.reserve(path.size());
    for (const char c : path) {
        if (c == '\\') out += '/';
        else out += c;
    }
    // optional: lowercase for Windows
    std::ranges::transform(out, out.begin(),
                           [](unsigned char c){ return std::tolower(c); });
    return out;
}

std::wstring CanonicalizePath(const std::wstring& path) {
    std::wstring out;
    out.reserve(path.size());
    for (const WCHAR c : path) {
        if (c == '\\') out += '/';
        else out += c;
    }
    return out;
}

void ResourceSharer::AddToDatabaseTex(const std::string& id, std::shared_ptr<Texture> tex)
{
    s_databaseTex.insert_or_assign(CanonicalizePath(id), tex);
}

bool ResourceSharer::TryGetFromDatabaseTex(const std::string& id, std::shared_ptr<Texture>& tex)
{
    const auto it = s_databaseTex.find(CanonicalizePath(id));
    if (it == s_databaseTex.end())
        return false;

    tex = it->second;
    return true;
}

void ResourceSharer::AddToDatabaseBindless(const std::wstring& id, uint32_t idx)
{
    s_databaseBindless.insert_or_assign(CanonicalizePath(id), idx);
}

bool ResourceSharer::TryGetFromDatabaseBindless(const std::wstring& id, uint32_t& idx)
{
    const auto it = s_databaseBindless.find(CanonicalizePath(id));
    if (it == s_databaseBindless.end())
        return false;

    idx = it->second;
    return true;
}
