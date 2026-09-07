#pragma once
#include "Core/Component.h"
#include <memory>
#include <vector>
#include <glm/glm.hpp>

class Texture;
class Shader;
class Mesh;
class Camera;
class Light;

class MeshRenderer : public Component {
public:
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Texture> diffuseMap;
    std::shared_ptr<Texture> specularMap;
    std::shared_ptr<Texture> normalMap;
    std::shared_ptr<Texture> heightMap;
    float heightScale = 0.05f;   // 视差强度

    glm::vec2 uvTiling{ 1.0f, 1.0f };   // UV 平铺次数（贴图是 REPEAT 环绕）
    glm::vec3 color{ 0.8f, 0.5f, 0.3f };
    float shininess = 32.0f;

    bool transparent = false;   // 透明物体：跳过 G-Buffer，走后处理的前向 pass
    float alpha = 1.0f;

    MeshRenderer(std::shared_ptr<Mesh> m, std::shared_ptr<Shader> s)
        : mesh(std::move(m)), shader(std::move(s)) {
    }

    void DrawGeometry(Shader& shader);

    void Draw(const Camera& cam, float aspect, const std::vector<Light*>& lights);

    void DrawGBuffer(Shader& gbufferShader, const Camera& cam, float aspect);

    void BindMaterial(Shader& shader);   // 贴图绑定抽成公共方法
};