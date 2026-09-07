#pragma once
#include "Core/Component.h"
#include "Core/Object.h"
#include "Components/Transform.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <array>

class Framebuffer;

enum class LightType { Directional, Point };

class Light : public Component {
public:
    LightType type = LightType::Directional;
    glm::vec3 color{ 1.0f };
    float intensity = 1.0f;
    glm::vec3 direction{ -0.3f, -1.0f, -0.4f };

    // ---- 方向光阴影 ----
    bool castShadow = true;
    int shadowMapSize = 2048;
    std::shared_ptr<Framebuffer> shadowMap;
    glm::mat4 lightSpaceMatrix{ 1.0f };

    // ---- 点光源阴影（cubemap）----
    int pointShadowSize = 1024;
    float shadowFarPlane = 30.0f;    // 阴影覆盖半径
    std::shared_ptr<Framebuffer> shadowCubemap;

    glm::mat4 ComputeLightSpaceMatrix() const {
        glm::vec3 dir = glm::normalize(direction);
        glm::vec3 pos = -dir * 15.0f;
        glm::mat4 view = glm::lookAt(pos, glm::vec3(0.0f), { 0, 1, 0 });
        glm::mat4 proj = glm::ortho(-12.0f, 12.0f, -12.0f, 12.0f, 1.0f, 40.0f);
        return proj * view;
    }

    // 6 个方向的 view * projection（注意 +Y/-Y 面的 up 向量是特殊的）
    std::array<glm::mat4, 6> ComputePointShadowMatrices() const {
        glm::vec3 p = owner->transform->position;
        glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.5f, shadowFarPlane);
        return {
            proj * glm::lookAt(p, p + glm::vec3(1, 0, 0), { 0,-1, 0 }),
            proj * glm::lookAt(p, p + glm::vec3(-1, 0, 0), { 0,-1, 0 }),
            proj * glm::lookAt(p, p + glm::vec3(0, 1, 0), { 0, 0, 1 }),
            proj * glm::lookAt(p, p + glm::vec3(0,-1, 0), { 0, 0,-1 }),
            proj * glm::lookAt(p, p + glm::vec3(0, 0, 1), { 0,-1, 0 }),
            proj * glm::lookAt(p, p + glm::vec3(0, 0,-1), { 0,-1, 0 }),
        };
    }
};