//
// Created by fiona on 19/01/2026.
//

#ifndef CHERRYPIP_RESOURCESHARER_H
#define CHERRYPIP_RESOURCESHARER_H
#include <memory>
#include <string>
#include <unordered_map>


class Texture;

class ResourceSharer
{
public:
    static void AddToDatabase(const std::string& id, std::shared_ptr<Texture> tex);
    static bool TryGetFromDatabase(const std::string& id, std::shared_ptr<Texture>& tex);

private:
    static std::unordered_map<std::string, std::shared_ptr<Texture>> s_databaseTex;
};


#endif //CHERRYPIP_RESOURCESHARER_H