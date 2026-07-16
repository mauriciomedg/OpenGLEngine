#pragma once

#include <glad/glad.h>

class ScopedGpuDebugGroup
{
public:
    explicit ScopedGpuDebugGroup(const char* name)
    {
        if (glPushDebugGroup != nullptr && glPopDebugGroup != nullptr)
        {
            glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name);
            pushed_ = true;
        }
    }

    ~ScopedGpuDebugGroup()
    {
        if (pushed_)
        {
            glPopDebugGroup();
        }
    }

    ScopedGpuDebugGroup(const ScopedGpuDebugGroup&) = delete;
    ScopedGpuDebugGroup& operator=(const ScopedGpuDebugGroup&) = delete;

private:
    bool pushed_ = false;
};
