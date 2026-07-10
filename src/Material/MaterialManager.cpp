#include "MaterialManager.h"

#include <filesystem>

namespace
{
std::string normalizePath(const std::string& path)
{
    return std::filesystem::absolute(path).lexically_normal().string();
}
}

Material& MaterialManager::load(const std::string& materialPath)
{
    const std::string key = normalizePath(materialPath);
    const auto found = materials_.find(key);
    if (found != materials_.end())
    {
        return *found->second;
    }

    auto material = std::make_unique<Material>(key);
    Material& materialRef = *material;
    materials_.emplace(key, std::move(material));
    return materialRef;
}

Material* MaterialManager::find(const std::string& materialPath)
{
    const auto found = materials_.find(normalizePath(materialPath));
    return found != materials_.end() ? found->second.get() : nullptr;
}

const Material* MaterialManager::find(const std::string& materialPath) const
{
    const auto found = materials_.find(normalizePath(materialPath));
    return found != materials_.end() ? found->second.get() : nullptr;
}
