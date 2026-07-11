#include "Framebuffer.h"

#include "TextureFormat.h"

#include <glad/glad.h>

#include <stdexcept>

namespace
{
void deleteResources(unsigned int& fbo, unsigned int& texture, unsigned int& depthTexture)
{
    if (depthTexture != 0)
    {
        glDeleteTextures(1, &depthTexture);
        depthTexture = 0;
    }

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

void configureTextureSampling()
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void configureDepthTextureSampling()
{
    const float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
}
}

Framebuffer::Framebuffer(int width, int height, FramebufferFormat format)
    : Framebuffer(FramebufferDesc{width, height, true, false, format})
{
}

Framebuffer::Framebuffer(const FramebufferDesc& desc)
    : width_(desc.width)
    , height_(desc.height)
    , format_(desc.colorFormat)
{
    if (width_ <= 0 || height_ <= 0)
    {
        throw std::runtime_error("Framebuffer dimensions must be positive.");
    }

    if (!desc.colorAttachment && !desc.depthAttachment)
    {
        throw std::runtime_error("Framebuffer requires at least one attachment.");
    }

    glGenFramebuffers(1, &fbo_); // create a framebuffer object and put its ID inside fbo_ 
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_); // make this buffer the current render target

    if (desc.colorAttachment)
    {
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
            nullptr);

        configureTextureSampling();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_, 0);
    }

    if (desc.depthAttachment)
    {
        glGenTextures(1, &depthTexture_);
        glBindTexture(GL_TEXTURE_2D, depthTexture_);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_DEPTH_COMPONENT32F,
            width_,
            height_,
            0,
            GL_DEPTH_COMPONENT,
            GL_FLOAT,
            nullptr);

        configureDepthTextureSampling();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture_, 0);
    }

    if (!desc.colorAttachment)
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    // OpenGL framebuffers can be invalid if attachments are missing or formats are wrong.
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        deleteResources(fbo_, texture_, depthTexture_);
        throw std::runtime_error("Framebuffer is incomplete.");
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::~Framebuffer()
{
    deleteResources(fbo_, texture_, depthTexture_);
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : fbo_(other.fbo_)
    , texture_(other.texture_)
    , depthTexture_(other.depthTexture_)
    , width_(other.width_)
    , height_(other.height_)
    , format_(other.format_)
{
    other.fbo_ = 0;
    other.texture_ = 0;
    other.depthTexture_ = 0;
    other.width_ = 0;
    other.height_ = 0;
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
{
    if (this != &other)
    {
        deleteResources(fbo_, texture_, depthTexture_);

        fbo_ = other.fbo_;
        texture_ = other.texture_;
        depthTexture_ = other.depthTexture_;
        width_ = other.width_;
        height_ = other.height_;
        format_ = other.format_;

        other.fbo_ = 0;
        other.texture_ = 0;
        other.depthTexture_ = 0;
        other.width_ = 0;
        other.height_ = 0;
    }

    return *this;
}

void Framebuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glViewport(0, 0, width_, height_); // tells OpenGL where and how big to render inside the current framebuffer
}

void Framebuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int Framebuffer::textureId() const
{
    return texture_;
}

unsigned int Framebuffer::depthTextureId() const
{
    return depthTexture_;
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
