#pragma once

enum class FramebufferFormat
{
    RGBA8,
    R32F
};

class Framebuffer
{
public:
    Framebuffer(int width, int height, FramebufferFormat format);
    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    Framebuffer(Framebuffer&& other) noexcept;
    Framebuffer& operator=(Framebuffer&& other) noexcept;

    void bind() const;
    static void unbind();

    unsigned int textureId() const;
    unsigned int fboId() const;
    int width() const;
    int height() const;

private:
    unsigned int fbo_ = 0;
    unsigned int texture_ = 0;
    int width_ = 0;
    int height_ = 0;
    FramebufferFormat format_;
};
