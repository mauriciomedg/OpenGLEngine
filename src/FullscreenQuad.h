#pragma once

class FullscreenQuad
{
public:
    FullscreenQuad();
    ~FullscreenQuad();

    FullscreenQuad(const FullscreenQuad&) = delete;
    FullscreenQuad& operator=(const FullscreenQuad&) = delete;

    void draw() const;

private:
    unsigned int vao_ = 0;
    unsigned int vbo_ = 0;
};
