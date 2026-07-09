#pragma once

#include "Framebuffer.h"

class Texture2D
{
public:
    Texture2D(int width, int height, FramebufferFormat format, const void* data);
    ~Texture2D();

    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    Texture2D(Texture2D&& other) noexcept;
    Texture2D& operator=(Texture2D&& other) noexcept;

    void bind(unsigned int unit) const;

    unsigned int id() const;
    int width() const;
    int height() const;

private:
    unsigned int texture_ = 0;
    int width_ = 0;
    int height_ = 0;
    FramebufferFormat format_;
};
