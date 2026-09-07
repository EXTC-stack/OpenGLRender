#include "Core/Scene.h"
#include "Components/Camera.h"
#include "Components/Light.h"
#include "Components/MeshRenderer.h"
#include "Renderer/Shader.h"
#include "Renderer/Framebuffer.h"
#include "Renderer/GBuffer.h"
#include <glad/glad.h>
#include "Components/Transform.h"

Scene::~Scene() = default;

void Scene::Update(float dt) {
    for (auto& o : m_Objects) o->Update(dt);
}

// 给光照 shader 统一上灯光 uniform（阴影图绑到 3 号单元，0/1/2 被 G-Buffer 占用）
static void ApplyLightUniforms(Shader& s, const std::vector<Light*>& lights) {
    int numPoint = 0, dirSet = 0;
    for (auto* l : lights) {
        if (l->type == LightType::Directional) {
            s.SetVec3("dirLight.direction", l->direction);
            s.SetVec3("dirLight.color", l->color);
            s.SetFloat("dirLight.intensity", l->intensity);
            dirSet = 1;
            if (l->castShadow && l->shadowMap) {
                l->shadowMap->BindDepthTexture(3);
                s.SetInt("shadowMap", 3);
                s.SetMat4("lightSpaceMatrix", l->lightSpaceMatrix);
                s.SetInt("useShadow", 1);
            }
            else s.SetInt("useShadow", 0);
        }
        else if (numPoint < 32) {
            std::string base = "pointLights[" + std::to_string(numPoint) + "].";
            s.SetVec3(base + "position", l->owner->transform->position);
            s.SetVec3(base + "color", l->color);
            s.SetFloat(base + "intensity", l->intensity);
            numPoint++;
        }
    }
    s.SetInt("useDirLight", dirSet);
    s.SetInt("numPointLights", numPoint);

    // 找第一个开了阴影的点光源，把它的 cubemap 传进去
    int pointIndex = 0, shadowIndex = -1;
    for (auto* l : lights) {
        if (l->type != LightType::Point) continue;
        if (l->castShadow && l->shadowCubemap && shadowIndex == -1) {
            shadowIndex = pointIndex;
            l->shadowCubemap->BindDepthCubemap(4);   // 0/1/2=GBuffer, 3=方向光阴影
            s.SetInt("pointShadowMap", 4);
            s.SetVec3("pointShadowPos", l->owner->transform->position);
            s.SetFloat("pointFarPlane", l->shadowFarPlane);
        }
        pointIndex++;
    }
    s.SetInt("usePointShadow", shadowIndex >= 0 ? 1 : 0);
    s.SetInt("pointShadowIndex", shadowIndex);
}

void Scene::Render(int width, int height) {
    std::vector<Light*> lights;
    Camera* cam = mainCamera;
    for (auto& o : m_Objects) {
        if (auto* l = o->GetComponent<Light>()) lights.push_back(l);
        if (!cam) if (auto* c = o->GetComponent<Camera>()) cam = c;
    }
    if (!cam) return;

    if (!m_PointDepthShader)
        m_PointDepthShader = std::make_shared<Shader>(
            "shaders/shadow_point.vert", "shaders/shadow_point.frag", "shaders/shadow_point.geom");

    for (auto* l : lights) {
        if (l->type != LightType::Point || !l->castShadow) continue;
        if (!l->shadowCubemap)
            l->shadowCubemap = Framebuffer::CreateDepthCubemap(l->pointShadowSize);

        l->shadowCubemap->Bind();
        glClear(GL_DEPTH_BUFFER_BIT);
        m_PointDepthShader->Use();

        auto mats = l->ComputePointShadowMatrices();
        for (int i = 0; i < 6; i++)
            m_PointDepthShader->SetMat4("lightSpaceMatrices[" + std::to_string(i) + "]", mats[i]);
        m_PointDepthShader->SetVec3("lightPos", l->owner->transform->position);
        m_PointDepthShader->SetFloat("farPlane", l->shadowFarPlane);

        for (auto& o : m_Objects)
            if (auto* mr = o->GetComponent<MeshRenderer>())
                mr->DrawGeometry(*m_PointDepthShader);
    }

    if (!m_DepthShader)
        m_DepthShader = std::make_shared<Shader>("shaders/shadow_depth.vert", "shaders/shadow_depth.frag");

    // ---- Pass 0: 阴影深度（和前向一样）----
    for (auto* l : lights) {
        if (l->type != LightType::Directional || !l->castShadow) continue;
        if (!l->shadowMap)
            l->shadowMap = Framebuffer::CreateDepthOnly(l->shadowMapSize, l->shadowMapSize);

        l->lightSpaceMatrix = l->ComputeLightSpaceMatrix();
        l->shadowMap->Bind();
        glClear(GL_DEPTH_BUFFER_BIT);
        m_DepthShader->Use();
        m_DepthShader->SetMat4("lightSpaceMatrix", l->lightSpaceMatrix);
        for (auto& o : m_Objects)
            if (auto* mr = o->GetComponent<MeshRenderer>())
                mr->DrawGeometry(*m_DepthShader);
    }
    Framebuffer::Unbind(width, height);

    float aspect = (float)width / (float)height;

    if (useDeferred) {
        if (!m_GBufferShader)
            m_GBufferShader = std::make_shared<Shader>("shaders/gbuffer.vert", "shaders/gbuffer.frag");
        if (!m_DeferredShader)
            m_DeferredShader = std::make_shared<Shader>("shaders/deferred_lighting.vert", "shaders/deferred_lighting.frag");
        if (!m_GBuffer || m_GBuffer->width != width || m_GBuffer->height != height)
            m_GBuffer = GBuffer::Create(width, height);   // 窗口变化自动重建

        // ---- Pass 1: 几何 ----
        m_GBuffer->Bind();
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_GBufferShader->Use();
        for (auto& o : m_Objects)
            if (auto* mr = o->GetComponent<MeshRenderer>()) {
                if (mr->transparent) continue;
                mr->DrawGBuffer(*m_GBufferShader, *cam, aspect);
            }

        // ---- Pass 2: 光照 ----
        m_GBuffer->Unbind(width, height);
        glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_DeferredShader->Use();
        m_GBuffer->BindTextures(0);
        m_DeferredShader->SetInt("gPosition", 0);
        m_DeferredShader->SetInt("gNormal", 1);
        m_DeferredShader->SetInt("gAlbedoSpec", 2);
        m_DeferredShader->SetVec3("viewPos", cam->GetPosition());
        m_DeferredShader->SetFloat("shininess", 32.0f);
        ApplyLightUniforms(*m_DeferredShader, lights);
        GBuffer::DrawQuad();

        // ---- Pass 3: 深度回拷 ----
        // 光照 pass 画的是全屏 Quad，默认帧缓冲里没有场景深度，
        // 透明物体要正确遮挡必须把 G-Buffer 的深度拷回来
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_GBuffer->FBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
            GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ---- Pass 4: 透明物体（前向渲染）----
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);   // 透明只测深度、不写深度
        for (auto& o : m_Objects)
            if (auto* mr = o->GetComponent<MeshRenderer>())
                if (mr->transparent)
                    mr->Draw(*cam, aspect, lights);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
    else {
        // ---- 前向路径（保留对比）----
        for (auto& o : m_Objects)
            if (auto* mr = o->GetComponent<MeshRenderer>())
                mr->Draw(*cam, aspect, lights);
    }
}