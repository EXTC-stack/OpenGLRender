#pragma once
#include "Core/Component.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform : public Component {
public:
    glm::vec3 position{ 0.0f };
    glm::vec3 rotationEuler{ 0.0f };  // 单位：度
    glm::vec3 scale{ 1.0f };

    glm::mat4 GetModelMatrix() const {
        glm::mat4 m(1.0f);
        m = glm::translate(m, position);
        m = glm::rotate(m, glm::radians(rotationEuler.y), { 0, 1, 0 });
        m = glm::rotate(m, glm::radians(rotationEuler.x), { 1, 0, 0 });
        m = glm::rotate(m, glm::radians(rotationEuler.z), { 0, 0, 1 });
        m = glm::scale(m, scale);
        return m;
    }
};