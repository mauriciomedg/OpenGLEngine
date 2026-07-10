#pragma once

#include "Material.h"

#include <memory>
#include <string>
#include <unordered_map>

class MaterialManager
{
public:
    Material& load(const std::string& materialPath);

    Material* find(const std::string& materialPath);
    const Material* find(const std::string& materialPath) const;

private:
    std::unordered_map<std::string, std::unique_ptr<Material>> materials_;
};
