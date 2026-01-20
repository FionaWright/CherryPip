//
// Created by fiona on 19/01/2026.
//

#include "System/pch.h"
#include "System/ResourceSharer.h"

#include <ranges>

#include "Helper.h"

std::unordered_map<std::string, std::shared_ptr<Texture>> ResourceSharer::s_databaseTex;

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

void ResourceSharer::AddToDatabase(const std::string& id, std::shared_ptr<Texture> tex)
{
    s_databaseTex.insert_or_assign(CanonicalizePath(id), tex);
}

bool ResourceSharer::TryGetFromDatabase(const std::string& id, std::shared_ptr<Texture>& tex)
{
    const auto it = s_databaseTex.find(CanonicalizePath(id));
    if (it == s_databaseTex.end())
        return false;

    tex = it->second;
    return true;
}
