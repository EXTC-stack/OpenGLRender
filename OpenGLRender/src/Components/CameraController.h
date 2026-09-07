#pragma once
#include "Core/Component.h"

struct GLFWwindow;

class CameraController : public Component {
public:
    GLFWwindow* window = nullptr;
    float moveSpeed = 3.0f;
    float mouseSensitivity = 0.1f;

    void Update(float dt) override;

private:
    bool m_looking = false;
    double m_lastX = 0.0, m_lastY = 0.0;
};