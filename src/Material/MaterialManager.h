#pragma once

#include "Material.h"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

class MaterialManager
{
public:
    Material& load(const std::filesystem::path& materialPath);

    Material* find(const std::filesystem::path& materialPath);
    const Material* find(const std::filesystem::path& materialPath) const;

private:
    std::unordered_map<std::string, std::unique_ptr<Material>> materials_;
};
