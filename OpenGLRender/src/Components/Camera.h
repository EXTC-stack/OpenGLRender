#pragma once
#include "Core/Component.h"
#include "Core/Object.h"
#include "Components/Transform.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera : public Component {
public:
    float fov = 45.0f, nearPlane = 0.1f, farPlane = 100.0f;
    float yaw = -90.0f, pitch = 0.0f;

    glm::vec3 GetPosition() const { return owner->transform->position; }

    glm::vec3 GetFront() const {
        glm::vec3 f;
        f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        f.y = sin(glm::radians(pitch));
        f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        return glm::normalize(f);
    }

    glm::mat4 GetViewMatrix() const {
        glm::vec3 pos = GetPosition();
        return glm::lookAt(pos, pos + GetFront(), { 0, 1, 0 });
    }

    glm::mat4 GetProjectionMatrix(float aspect) const {
        return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
    }
};