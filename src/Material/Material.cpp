#include "Material.h"

#include <tinyxml2.h>

#include <filesystem>
#include <stdexcept>

namespace
{
std::filesystem::path normalizePath(const std::filesystem::path& path)
{
    return std::filesystem::absolute(path).lexically_normal();
}

std::filesystem::path resolveRelativePath(const std::filesystem::path& materialPath, const char* value)
{
    return normalizePath(materialPath.parent_path() / value);
}
}

Material::Material(const std::string& materialPath)
    : sourcePath_(normalizePath(materialPath).string())
{
    tinyxml2::XMLDocument document;
    const tinyxml2::XMLError loadResult = document.LoadFile(sourcePath_.c_str());
    if (loadResult != tinyxml2::XML_SUCCESS)
    {
        throw std::runtime_error("Failed to load material XML: " + sourcePath_);
    }

    const tinyxml2::XMLElement* material = document.FirstChildElement("Material");
    if (material == nullptr)
    {
        throw std::runtime_error("Material XML is missing <Material>: " + sourcePath_);
    }

    const char* name = material->Attribute("name");
    name_ = name != nullptr ? name : "";

    const tinyxml2::XMLElement* shader = material->FirstChildElement("Shader");
    if (shader == nullptr)
    {
        throw std::runtime_error("Material XML is missing <Shader>: " + sourcePath_);
    }

    const char* vertex = shader->Attribute("vertex");
    if (vertex == nullptr || vertex[0] == '\0')
    {
        throw std::runtime_error("Material Shader is missing vertex attribute: " + sourcePath_);
    }

    const char* fragment = shader->Attribute("fragment");
    if (fragment == nullptr || fragment[0] == '\0')
    {
        throw std::runtime_error("Material Shader is missing fragment attribute: " + sourcePath_);
    }

    const std::filesystem::path sourcePath(sourcePath_);
    const std::filesystem::path vertexPath = resolveRelativePath(sourcePath, vertex);
    const std::filesystem::path fragmentPath = resolveRelativePath(sourcePath, fragment);
    shader_ = std::make_unique<Shader>(vertexPath.string(), fragmentPath.string());
}

Shader& Material::shader()
{
    return *shader_;
}

const Shader& Material::shader() const
{
    return *shader_;
}

const std::string& Material::name() const
{
    return name_;
}

const std::string& Material::sourcePath() const
{
    return sourcePath_;
}
