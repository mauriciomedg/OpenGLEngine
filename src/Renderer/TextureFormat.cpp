#include "TextureFormat.h"

#include <glad/glad.h>

#include <stdexcept>

TextureFormatInfo toTextureFormatInfo(FramebufferFormat format)
{
    switch (format)
    {
    case FramebufferFormat::RGBA8:
        return {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE};
    case FramebufferFormat::R32F:
        return {GL_R32F, GL_RED, GL_FLOAT};
    }

    throw std::runtime_error("Unsupported texture format.");
}
