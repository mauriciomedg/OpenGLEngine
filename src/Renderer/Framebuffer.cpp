#include "Framebuffer.h"

#include <glad/glad.h>

#include <stdexcept>

namespace
{
struct TextureFormat
{
    int internalFormat;
    unsigned int format;
    unsigned int type;
};

TextureFormat toTextureFormat(FramebufferFormat format)
{
    switch (format)
    {
    case FramebufferFormat::RGBA8:
        return {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE};
    case FramebufferFormat::R32F:
        return {GL_R32F, GL_RED, GL_FLOAT};
    }

    throw std::runtime_error("Unsupported framebuffer format.");
}

void deleteResources(unsigned int& fbo, unsigned int& texture)
{
    if (texture != 0)
    {
        glDeleteTextures(1, &texture);
        texture = 0;
    }

    if (fbo != 0)
    {
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
    }
}
}

Framebuffer::Framebuffer(int width, int height, FramebufferFormat format)
    : width_(width)
    , height_(height)
    , format_(format)
{
    if (width_ <= 0 || height_ <= 0)
    {
        throw std::runtime_error("Framebuffer dimensions must be positive.");
    }

    const TextureFormat textureFormat = toTextureFormat(format_);

    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

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
        nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        deleteResources(fbo_, texture_);
        throw std::runtime_error("Framebuffer is incomplete.");
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::~Framebuffer()
{
    deleteResources(fbo_, texture_);
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : fbo_(other.fbo_)
    , texture_(other.texture_)
    , width_(other.width_)
    , height_(other.height_)
    , format_(other.format_)
{
    other.fbo_ = 0;
    other.texture_ = 0;
    other.width_ = 0;
    other.height_ = 0;
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
{
    if (this != &other)
    {
        deleteResources(fbo_, texture_);

        fbo_ = other.fbo_;
        texture_ = other.texture_;
        width_ = other.width_;
        height_ = other.height_;
        format_ = other.format_;

        other.fbo_ = 0;
        other.texture_ = 0;
        other.width_ = 0;
        other.height_ = 0;
    }

    return *this;
}

void Framebuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
}

void Framebuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int Framebuffer::textureId() const
{
    return texture_;
}

unsigned int Framebuffer::fboId() const
{
    return fbo_;
}

int Framebuffer::width() const
{
    return width_;
}

int Framebuffer::height() const
{
    return height_;
}
