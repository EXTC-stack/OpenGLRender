#pragma once
#include <vector>
#include <memory>
#include "Core/Object.h"
#include "Renderer/GBuffer.h"

class Camera;
class Shader;
class GBuffer;


class Scene {
public:
    bool useDeferred = true;
    std::shared_ptr<Shader> m_PointDepthShader;

    Object& Add(std::unique_ptr<Object> obj) {
        m_Objects.push_back(std::move(obj));
        return *m_Objects.back();
    }

    void Update(float dt);
    void Render(int width, int height);

    Camera* mainCamera = nullptr;

    ~Scene();

private:
    std::vector<std::unique_ptr<Object>> m_Objects;
    std::shared_ptr<Shader> m_DepthShader;
    std::shared_ptr<Shader> m_GBufferShader;
    std::shared_ptr<Shader> m_DeferredShader;
    std::unique_ptr<GBuffer> m_GBuffer;
};