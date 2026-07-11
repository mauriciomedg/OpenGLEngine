#include "Mesh.h"

#include <glad/glad.h>

namespace
{
void deleteResources(unsigned int& vao, unsigned int& vbo, unsigned int& ebo)
{
    if (ebo != 0)
    {
        glDeleteBuffers(1, &ebo);
        ebo = 0;
    }

    if (vbo != 0)
    {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }

    if (vao != 0)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
}
}

Mesh::Mesh(const float* vertices, int vertexFloatCount, const unsigned int* indices, int indexCount)
    : indexCount_(indexCount)
{
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertexFloatCount * sizeof(float)), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indexCount_ * sizeof(unsigned int)), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    constexpr int Stride = 9;

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, Stride * sizeof(float), nullptr);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, Stride * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, Stride * sizeof(float), reinterpret_cast<void*>(6 * sizeof(float)));

    glBindVertexArray(0);
}

Mesh::~Mesh()
{
    deleteResources(vao_, vbo_, ebo_);
}

Mesh::Mesh(Mesh&& other) noexcept
    : vao_(other.vao_)
    , vbo_(other.vbo_)
    , ebo_(other.ebo_)
    , indexCount_(other.indexCount_)
{
    other.vao_ = 0;
    other.vbo_ = 0;
    other.ebo_ = 0;
    other.indexCount_ = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    if (this != &other)
    {
        deleteResources(vao_, vbo_, ebo_);

        vao_ = other.vao_;
        vbo_ = other.vbo_;
        ebo_ = other.ebo_;
        indexCount_ = other.indexCount_;

        other.vao_ = 0;
        other.vbo_ = 0;
        other.ebo_ = 0;
        other.indexCount_ = 0;
    }

    return *this;
}

Mesh Mesh::createCube()
{
    const float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.90f, 0.18f, 0.22f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.95f, 0.62f, 0.18f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.20f, 0.72f, 0.36f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.12f, 0.56f, 0.92f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.72f, 0.24f, 0.88f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.10f, 0.78f, 0.82f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.86f, 0.82f, 0.20f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.84f, 0.32f, 0.52f,

        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.90f, 0.18f, 0.22f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.72f, 0.24f, 0.88f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.84f, 0.32f, 0.52f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.12f, 0.56f, 0.92f,

         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.95f, 0.62f, 0.18f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.10f, 0.78f, 0.82f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.86f, 0.82f, 0.20f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.20f, 0.72f, 0.36f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.12f, 0.56f, 0.92f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.20f, 0.72f, 0.36f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.86f, 0.82f, 0.20f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.84f, 0.32f, 0.52f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.90f, 0.18f, 0.22f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.95f, 0.62f, 0.18f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.10f, 0.78f, 0.82f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.72f, 0.24f, 0.88f,
    };

    const unsigned int indices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20,
    };

    return Mesh(vertices, 216, indices, 36);
}

Mesh Mesh::createPlane()
{
    const float vertices[] = {
        -4.0f, -1.0f, -4.0f, 0.0f, 1.0f, 0.0f, 0.58f, 0.58f, 0.56f,
         4.0f, -1.0f, -4.0f, 0.0f, 1.0f, 0.0f, 0.58f, 0.58f, 0.56f,
         4.0f, -1.0f,  4.0f, 0.0f, 1.0f, 0.0f, 0.58f, 0.58f, 0.56f,
        -4.0f, -1.0f,  4.0f, 0.0f, 1.0f, 0.0f, 0.58f, 0.58f, 0.56f,
    };

    const unsigned int indices[] = {
        0, 1, 2, 2, 3, 0,
    };

    return Mesh(vertices, 36, indices, 6);
}

void Mesh::draw() const
{
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}
