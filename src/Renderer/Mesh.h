#pragma once

#include <cstddef>
#include <vector>

#include <glm/mat4x4.hpp>

class Mesh
{
public:
    Mesh() = default;
    Mesh(const float* vertices, int vertexFloatCount, const unsigned int* indices, int indexCount);
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    static Mesh createCube();
    static Mesh createPlane();

    void draw() const;
    void drawInstanced(std::size_t instanceCount) const;
    void setInstanceTransforms(const std::vector<glm::mat4>& transforms);

private:
    unsigned int vao_ = 0;
    unsigned int vbo_ = 0;
    unsigned int ebo_ = 0;
    unsigned int instanceVbo_ = 0;
    int indexCount_ = 0;
};
