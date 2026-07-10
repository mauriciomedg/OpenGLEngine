#pragma once

enum class FramebufferFormat
{
    RGBA8,
    R32F,
    Depth32F
};

struct FramebufferDesc
{
    int width = 0;
    int height = 0;
    bool colorAttachment = true;
    bool depthAttachment = false;
    FramebufferFormat colorFormat = FramebufferFormat::RGBA8;
};

class Framebuffer
{
public:
    Framebuffer(int width, int height, FramebufferFormat format);
    explicit Framebuffer(const FramebufferDesc& desc);
    ~Framebuffer();

    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    Framebuffer(Framebuffer&& other) noexcept;
    Framebuffer& operator=(Framebuffer&& other) noexcept;

    void bind() const;
    static void unbind();

    unsigned int textureId() const;
    unsigned int depthTextureId() const;
    unsigned int fboId() const;
    int width() const;
    int height() const;

private:
    unsigned int fbo_ = 0;
    unsigned int texture_ = 0;
    unsigned int depthTexture_ = 0;
    int width_ = 0;
    int height_ = 0;
    FramebufferFormat format_;
};
