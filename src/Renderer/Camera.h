#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class Camera
{
public:
    Camera(float aspectRatio);

    glm::mat4 viewMatrix() const;
    glm::mat4 projectionMatrix() const;

    void setAspectRatio(float aspectRatio);

private:
    glm::vec3 position_ = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 target_ = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up_ = glm::vec3(0.0f, 1.0f, 0.0f);
    float fovDegrees_ = 45.0f;
    float aspectRatio_ = 1.0f;
    float nearPlane_ = 0.1f;
    float farPlane_ = 100.0f;
};
