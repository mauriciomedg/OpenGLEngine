#include "Camera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

Camera::Camera(float aspectRatio)
    : aspectRatio_(aspectRatio)
{
}

glm::mat4 Camera::viewMatrix() const
{
    return glm::lookAt(position_, target_, up_);
}

glm::mat4 Camera::projectionMatrix() const
{
    return glm::perspective(glm::radians(fovDegrees_), aspectRatio_, nearPlane_, farPlane_);
}

void Camera::setAspectRatio(float aspectRatio)
{
    aspectRatio_ = aspectRatio;
}

void Camera::setView(const glm::vec3& position, const glm::vec3& target)
{
    position_ = position;
    target_ = target;
}

void Camera::setPerspective(float fovDegrees, float nearPlane, float farPlane)
{
    fovDegrees_ = fovDegrees;
    nearPlane_ = nearPlane;
    farPlane_ = farPlane;
}
