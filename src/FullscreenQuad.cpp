#include "FullscreenQuad.h"

#include <glad/glad.h>

FullscreenQuad::FullscreenQuad()
{
    const float vertices[] = {
        -1.0f, -1.0f,
         3.0f, -1.0f,
        -1.0f,  3.0f,
    };

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

FullscreenQuad::~FullscreenQuad()
{
    if (vbo_ != 0)
    {
        glDeleteBuffers(1, &vbo_);
    }

    if (vao_ != 0)
    {
        glDeleteVertexArrays(1, &vao_);
    }
}

void FullscreenQuad::draw() const
{
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}
