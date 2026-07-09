#pragma once

#include <string>

class Shader
{
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void use() const;

    void setInt(const char* name, int value) const;

private:
    unsigned int program_ = 0;
};
