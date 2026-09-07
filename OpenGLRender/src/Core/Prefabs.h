#pragma once
#include <memory>
#include <glm/glm.hpp>

class Scene;
class Object;
class Mesh;
class Shader;
struct GLFWwindow;
class Texture;

namespace Prefabs {
    Object* CreateCube(Scene& scene, std::shared_ptr<Mesh> mesh, std::shared_ptr<Shader> shader,
        glm::vec3 position = { 0, 0, 0 },
        std::shared_ptr<Texture> diffuse = nullptr,
        std::shared_ptr<Texture> specular = nullptr,
        std::shared_ptr<Texture> normal = nullptr,
        std::shared_ptr<Texture> height = nullptr);

    Object* CreateDirectionalLight(Scene& scene, glm::vec3 direction,
        glm::vec3 color = { 1, 1, 1 }, float intensity = 1.0f);

    Object* CreatePointLight(Scene& scene, glm::vec3 position,
        glm::vec3 color, float intensity = 1.0f);

    Object* CreateMainCamera(Scene& scene, GLFWwindow* window, glm::vec3 position = { 0, 1, 3.5f });

    // 通用积木：一个可缩放/染色/贴图的立方体
    Object* CreateBox(Scene& scene, std::shared_ptr<Mesh> mesh, std::shared_ptr<Shader> shader,
        glm::vec3 position, glm::vec3 scale, glm::vec3 color,
        std::shared_ptr<Texture> diffuse = nullptr,
        std::shared_ptr<Texture> normal = nullptr,
        std::shared_ptr<Texture> height = nullptr,
        glm::vec2 uvTiling = { 1.0f, 1.0f });

    // 组合家具（内部就是一堆 CreateBox）
    Object* CreateTable(Scene& scene, std::shared_ptr<Mesh> mesh, std::shared_ptr<Shader> shader,
        glm::vec3 position, std::shared_ptr<Texture> wood = nullptr);
    Object* CreateChair(Scene& scene, std::shared_ptr<Mesh> mesh, std::shared_ptr<Shader> shader,
        glm::vec3 position, float yawDegrees = 0.0f, std::shared_ptr<Texture> wood = nullptr);
}