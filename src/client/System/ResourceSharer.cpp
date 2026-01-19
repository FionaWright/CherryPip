//
// Created by fiona on 19/01/2026.
//

#include "System/ResourceSharer.h"

std::unordered_map<std::string, std::shared_ptr<Texture>> ResourceSharer::s_databaseTex;

void ResourceSharer::AddToDatabase(const std::string& id, std::shared_ptr<Texture> tex)
{
    s_databaseTex.insert_or_assign(id, tex);
}

bool ResourceSharer::TryGetFromDatabase(const std::string& id, std::shared_ptr<Texture>& tex)
{
    if (!s_databaseTex.contains(id))
        return false;

    tex = s_databaseTex[id];
    return true;
}
