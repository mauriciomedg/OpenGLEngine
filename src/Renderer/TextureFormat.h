#pragma once

#include "Framebuffer.h"

struct TextureFormatInfo
{
    int internalFormat;
    unsigned int format;
    unsigned int type;
};

TextureFormatInfo toTextureFormatInfo(FramebufferFormat format);
