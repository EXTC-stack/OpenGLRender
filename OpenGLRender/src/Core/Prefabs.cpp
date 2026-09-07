#include "Core/Prefabs.h"
#include "Core/Scene.h"
#include "Core/Object.h"
#include "Components/Transform.h"
#include "Components/Camera.h"
#include "Components/CameraController.h"
#include "Components/Light.h"
#include "Components/MeshRenderer.h"
#include <glm/gtc/matrix_transform.hpp>

Object* Prefabs::CreateCube(Scene& scene, std::shared_ptr<Mesh> mesh,
    std::shared_ptr<Shader> shader, glm::vec3 position,
    std::shared_ptr<Texture> diffuse,
    std::shared_ptr<Texture> specular,
    std::shared_ptr<Texture> normal,      // ← 新加的
    std::shared_ptr<Texture> height) {    // ← 新加的
    auto obj = std::make_unique<Object>("Cube");
    obj->transform->position = position;
    auto* mr = obj->AddComponent<MeshRenderer>(std::move(mesh), std::move(shader));
    mr->diffuseMap = std::move(diffuse);
    mr->specularMap = std::move(specular);
    mr->normalMap = std::move(normal);
    mr->heightMap = std::move(height);
    Object* ptr = obj.get();
    scene.Add(std::move(obj));
    return ptr;
}

Object* Prefabs::CreateDirectionalLight(Scene& scene, glm::vec3 direction,
    glm::vec3 color, float intensity) {
    auto obj = std::make_unique<Object>("DirectionalLight");
    auto* light = obj->AddComponent<Light>();
    light->type = LightType::Directional;
    light->direction = direction;
    light->color = color;
    light->intensity = intensity;
    Object* ptr = obj.get();
    scene.Add(std::move(obj));
    return ptr;
}

Object* Prefabs::CreatePointLight(Scene& scene, glm::vec3 position,
    glm::vec3 color, float intensity) {
    auto obj = std::make_unique<Object>("PointLight");
    obj->transform->position = position;
    auto* light = obj->AddComponent<Light>();
    light->type = LightType::Point;
    light->color = color;
    light->intensity = intensity;
    Object* ptr = obj.get();
    scene.Add(std::move(obj));
    return ptr;
}

Object* Prefabs::CreateMainCamera(Scene& scene, GLFWwindow* window, glm::vec3 position) {
    auto obj = std::make_unique<Object>("MainCamera");
    obj->transform->position = position;
    auto* cam = obj->AddComponent<Camera>();
    cam->pitch = -15.0f;
    auto* ctrl = obj->AddComponent<CameraController>();
    ctrl->window = window;
    scene.mainCamera = cam;
    Object* ptr = obj.get();
    scene.Add(std::move(obj));
    return ptr;
}

Object* Prefabs::CreateBox(Scene& scene, std::shared_ptr<Mesh> mesh, std::shared_ptr<Shader> shader,
    glm::vec3 position, glm::vec3 scale, glm::vec3 color,
    std::shared_ptr<Texture> diffuse ,
    std::shared_ptr<Texture> normal ,
    std::shared_ptr<Texture> height,
    glm::vec2 uvTiling ) {
    auto obj = std::make_unique<Object>("Box");
    obj->transform->position = position;
    obj->transform->scale = scale;
    auto* mr = obj->AddComponent<MeshRenderer>(std::move(mesh), std::move(shader));
    mr->color = color;
    mr->diffuseMap = std::move(diffuse);
    mr->normalMap = std::move(normal);
    mr->heightMap = std::move(height);
    mr->uvTiling = uvTiling;
    Object* ptr = obj.get();
    scene.Add(std::move(obj));
    return ptr;
}

Object* Prefabs::CreateTable(Scene& scene, std::shared_ptr<Mesh> mesh,
    std::shared_ptr<Shader> shader, glm::vec3 pos,
    std::shared_ptr<Texture> wood) {
    glm::vec3 c = { 0.45f, 0.30f, 0.18f };  // 木色
    // 桌面
    Object* top = CreateBox(scene, mesh, shader, pos + glm::vec3(0, 0.74f, 0), { 1.6f, 0.08f, 0.9f }, c, wood);
    // 四条腿
    for (int i = 0; i < 4; i++) {
        float x = (i % 2 == 0) ? -0.72f : 0.72f;
        float z = (i < 2) ? -0.37f : 0.37f;
        CreateBox(scene, mesh, shader, pos + glm::vec3(x, 0.37f, z), { 0.08f, 0.74f, 0.08f }, c, wood);
    }
    return top;
}

Object* Prefabs::CreateChair(Scene& scene, std::shared_ptr<Mesh> mesh,
    std::shared_ptr<Shader> shader, glm::vec3 pos,
    float yawDegrees, std::shared_ptr<Texture> wood) {
    glm::vec3 c = { 0.40f, 0.26f, 0.15f };
    float yaw = glm::radians(yawDegrees);
    // 局部偏移量绕 Y 轴旋转，让椅子能朝向任意方向
    auto rot = [&](glm::vec3 v) {
        return glm::vec3(v.x * cos(yaw) + v.z * sin(yaw), v.y, -v.x * sin(yaw) + v.z * cos(yaw));
        };
    auto part = [&](glm::vec3 offset, glm::vec3 scale) {
        Object* o = CreateBox(scene, mesh, shader, pos + rot(offset), scale, c, wood);
        o->transform->rotationEuler.y = yawDegrees;
        return o;
        };
    // 座面
    Object* seat = part({ 0, 0.45f, 0 }, { 0.45f, 0.06f, 0.45f });
    // 四条腿
    for (int i = 0; i < 4; i++) {
        float x = (i % 2 == 0) ? -0.19f : 0.19f;
        float z = (i < 2) ? -0.19f : 0.19f;
        part({ x, 0.225f, z }, { 0.06f, 0.45f, 0.06f });
    }
    // 靠背（在局部 -z 一侧）
    part({ 0, 0.73f, -0.20f }, { 0.45f, 0.55f, 0.06f });
    return seat;
}