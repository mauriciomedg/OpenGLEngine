#include "Texture2D.h"

#include "TextureFormat.h"

#include <glad/glad.h>

#include <stdexcept>

namespace
{
void deleteTexture(unsigned int& texture)
{
    if (texture != 0)
    {
        glDeleteTextures(1, &texture);
        texture = 0;
    }
}
}

Texture2D::Texture2D(int width, int height, FramebufferFormat format, const void* data)
    : width_(width)
    , height_(height)
    , format_(format)
{
    if (width_ <= 0 || height_ <= 0)
    {
        throw std::runtime_error("Texture dimensions must be positive.");
    }

    const TextureFormatInfo textureFormat = toTextureFormatInfo(format_);

    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        textureFormat.internalFormat,
        width_,
        height_,
        0,
        textureFormat.format,
        textureFormat.type,
        data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture2D::~Texture2D()
{
    deleteTexture(texture_);
}

Texture2D::Texture2D(Texture2D&& other) noexcept
    : texture_(other.texture_)
    , width_(other.width_)
    , height_(other.height_)
    , format_(other.format_)
{
    other.texture_ = 0;
    other.width_ = 0;
    other.height_ = 0;
}

Texture2D& Texture2D::operator=(Texture2D&& other) noexcept
{
    if (this != &other)
    {
        deleteTexture(texture_);

        texture_ = other.texture_;
        width_ = other.width_;
        height_ = other.height_;
        format_ = other.format_;

        other.texture_ = 0;
        other.width_ = 0;
        other.height_ = 0;
    }

    return *this;
}

void Texture2D::bind(unsigned int unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit); // chose the slot
    glBindTexture(GL_TEXTURE_2D, texture_); // put the texture in that slot
    //sampler uniform = tell shader which slot to read
}

void Texture2D::unbind(unsigned int unit)
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, 0);
}

unsigned int Texture2D::id() const
{
    return texture_;
}

int Texture2D::width() const
{
    return width_;
}

int Texture2D::height() const
{
    return height_;
}
