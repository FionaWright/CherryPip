//
// Created by fiona on 19/01/2026.
//

#ifndef CHERRYPIP_RESOURCESHARER_H
#define CHERRYPIP_RESOURCESHARER_H

class Texture;

class ResourceSharer
{
public:
    static void AddToDatabaseTex(const std::string& id, std::shared_ptr<Texture> tex);
    static bool TryGetFromDatabaseTex(const std::string& id, std::shared_ptr<Texture>& tex);

    static void AddToDatabaseBindless(const std::wstring& id, uint32_t idx);
    static bool TryGetFromDatabaseBindless(const std::wstring& id, uint32_t& idx);

private:
    static std::unordered_map<std::string, std::shared_ptr<Texture>> s_databaseTex;
    static std::unordered_map<std::wstring, uint32_t> s_databaseBindless;
};


#endif //CHERRYPIP_RESOURCESHARER_H