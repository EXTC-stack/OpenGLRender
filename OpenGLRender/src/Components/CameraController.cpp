#include "Components/CameraController.h"
#include "Core/Object.h"
#include "Components/Transform.h"
#include "Components/Camera.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

void CameraController::Update(float dt) {
    if (!window) return;
    auto* cam = owner->GetComponent<Camera>();
    if (!cam) return;

    // ---- 鼠标视角：按住右键拖动 ----
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        if (!m_looking) {
            m_looking = true;
            m_lastX = x; m_lastY = y;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        cam->yaw += (float)(x - m_lastX) * mouseSensitivity;
        cam->pitch = glm::clamp(cam->pitch + (float)(m_lastY - y) * mouseSensitivity, -89.0f, 89.0f);
        m_lastX = x; m_lastY = y;
    }
    else if (m_looking) {
        m_looking = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    // ---- 键盘移动：WASD + QE 升降，Shift 加速 ----
    float speed = moveSpeed * dt;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) speed *= 3.0f;

    glm::vec3 front = cam->GetFront();
    glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));
    glm::vec3& pos = owner->transform->position;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) pos += front * speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) pos -= front * speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) pos -= right * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) pos += right * speed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) pos.y += speed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) pos.y -= speed;
}