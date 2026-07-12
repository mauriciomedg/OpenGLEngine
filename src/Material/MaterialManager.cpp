#include "MaterialManager.h"

#include "../Debug/LoggingConfig.h"

#include <filesystem>
#include <iostream>

namespace
{
std::string normalizePath(const std::filesystem::path& path)
{
    return std::filesystem::absolute(path).lexically_normal().string();
}

std::string displayPath(const std::filesystem::path& path)
{
    const std::string normalized = std::filesystem::absolute(path).lexically_normal().generic_string();
    const std::string assetMarker = "/assets/";
    const std::size_t assetPosition = normalized.find(assetMarker);
    if (assetPosition != std::string::npos)
    {
        return normalized.substr(assetPosition + assetMarker.size());
    }

    return normalized;
}
}

Material& MaterialManager::load(const std::filesystem::path& materialPath)
{
    const std::string key = normalizePath(materialPath);
    const auto found = materials_.find(key);
    if (found != materials_.end())
    {
        if constexpr (EnableMaterialCacheLogging)
        {
            std::cout << "[Material] Cache hit " << displayPath(materialPath) << '\n';
        }
        return *found->second;
    }

    if constexpr (EnableMaterialCacheLogging)
    {
        std::cout << "[Material] Loading " << displayPath(materialPath) << '\n';
    }
    auto material = std::make_unique<Material>(key);
    Material& materialRef = *material;
    materials_.emplace(key, std::move(material));
    return materialRef;
}

Material* MaterialManager::find(const std::filesystem::path& materialPath)
{
    const auto found = materials_.find(normalizePath(materialPath));
    return found != materials_.end() ? found->second.get() : nullptr;
}

const Material* MaterialManager::find(const std::filesystem::path& materialPath) const
{
    const auto found = materials_.find(normalizePath(materialPath));
    return found != materials_.end() ? found->second.get() : nullptr;
}
