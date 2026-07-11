#include "Shader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace
{
std::string readTextFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file)
    {
        throw std::runtime_error("Failed to open shader file: " + path);
    }

    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

unsigned int compileShader(unsigned int type, const std::string& source, const std::string& label)
{
    const unsigned int shader = glCreateShader(type);
    const char* sourceText = source.c_str();
    glShaderSource(shader, 1, &sourceText, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[1024] = {};
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        glDeleteShader(shader);
        throw std::runtime_error("Failed to compile shader " + label + ": " + infoLog);
    }

    return shader;
}
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
{
    const std::string vertexSource = readTextFile(vertexPath);
    const std::string fragmentSource = readTextFile(fragmentPath);

    const unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource, vertexPath);
    const unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource, fragmentPath);

    program_ = glCreateProgram();
    glAttachShader(program_, vertexShader);
    glAttachShader(program_, fragmentShader);
    glLinkProgram(program_);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    int success = 0;
    glGetProgramiv(program_, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[1024] = {};
        glGetProgramInfoLog(program_, sizeof(infoLog), nullptr, infoLog);
        glDeleteProgram(program_);
        program_ = 0;
        throw std::runtime_error("Failed to link shader program: " + std::string(infoLog));
    }
}

Shader::~Shader()
{
    if (program_ != 0)
    {
        glDeleteProgram(program_);
    }
}

void Shader::setInt(const char* name, int value) const
{
    const int location = glGetUniformLocation(program_, name);
    if (location == -1)
    {
        throw std::runtime_error(std::string("Uniform not found: ") + name);
    }

    glUniform1i(location, value);
}

void Shader::setBool(const char* name, bool value) const
{
    setInt(name, value ? 1 : 0);
}

void Shader::setFloat(const char* name, float value) const
{
    const int location = glGetUniformLocation(program_, name);
    if (location == -1)
    {
        throw std::runtime_error(std::string("Uniform not found: ") + name);
    }

    glUniform1f(location, value);
}

void Shader::setMat4(const char* name, const glm::mat4& value) const
{
    const int location = glGetUniformLocation(program_, name);
    if (location == -1)
    {
        throw std::runtime_error(std::string("Uniform not found: ") + name);
    }

    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setVec3(const char* name, const glm::vec3& value) const
{
    const int location = glGetUniformLocation(program_, name);
    if (location == -1)
    {
        throw std::runtime_error(std::string("Uniform not found: ") + name);
    }

    glUniform3fv(location, 1, glm::value_ptr(value));
}

void Shader::use() const
{
    glUseProgram(program_);
}
