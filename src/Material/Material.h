#pragma once

#include "../Shader.h"

#include <memory>
#include <string>

class Material
{
public:
    explicit Material(const std::string& materialPath);

    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;

    Shader& shader();
    const Shader& shader() const;

    const std::string& name() const;
    const std::string& sourcePath() const;

private:
    std::string name_;
    std::string sourcePath_;
    std::unique_ptr<Shader> shader_;
};
