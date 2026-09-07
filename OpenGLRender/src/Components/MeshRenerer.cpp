#include "Components/MeshRenderer.h"
#include "Core/Object.h"
#include "Components/Transform.h"
#include "Components/Camera.h"
#include "Components/Light.h"
#include "Renderer/Shader.h"
#include "Renderer/Mesh.h"
#include "Renderer/Texture.h"
#include "Renderer/Framebuffer.h"

void MeshRenderer::BindMaterial(Shader& s) {
    s.SetVec3("objectColor", color);
    s.SetFloat("shininess", shininess);
    s.SetVec2("uvTiling", uvTiling);
    s.SetFloat("objectAlpha", alpha);

    if (diffuseMap) { diffuseMap->Bind(0); s.SetInt("diffuseMap", 0); s.SetInt("useDiffuseMap", 1); }
    else s.SetInt("useDiffuseMap", 0);

    if (specularMap) { specularMap->Bind(1); s.SetInt("specularMap", 1); s.SetInt("useSpecularMap", 1); }
    else s.SetInt("useSpecularMap", 0);

    if (normalMap) { normalMap->Bind(3); s.SetInt("normalMap", 3); s.SetInt("useNormalMap", 1); }
    else s.SetInt("useNormalMap", 0);

    if (heightMap) {
        heightMap->Bind(4); s.SetInt("heightMap", 4);
        s.SetInt("useHeightMap", 1); s.SetFloat("heightScale", heightScale);
    }
    else s.SetInt("useHeightMap", 0);
}

// 前向路径（保留，以后透明物体用）
void MeshRenderer::Draw(const Camera& cam, float aspect, const std::vector<Light*>& lights) {
    shader->Use();
    shader->SetMat4("model", owner->transform->GetModelMatrix());
    shader->SetMat4("view", cam.GetViewMatrix());
    shader->SetMat4("projection", cam.GetProjectionMatrix(aspect));
    shader->SetVec3("viewPos", cam.GetPosition());
    BindMaterial(*shader);

    int numPoint = 0, dirSet = 0;
    for (auto* l : lights) {
        if (l->type == LightType::Directional) {
            shader->SetVec3("dirLight.direction", l->direction);
            shader->SetVec3("dirLight.color", l->color);
            shader->SetFloat("dirLight.intensity", l->intensity);
            dirSet = 1;
            if (l->castShadow && l->shadowMap) {
                l->shadowMap->BindDepthTexture(2);
                shader->SetInt("shadowMap", 2);
                shader->SetMat4("lightSpaceMatrix", l->lightSpaceMatrix);
                shader->SetInt("useShadow", 1);
            }
            else shader->SetInt("useShadow", 0);
        }
        else if (numPoint < 8) {
            std::string base = "pointLights[" + std::to_string(numPoint) + "].";
            shader->SetVec3(base + "position", l->owner->transform->position);
            shader->SetVec3(base + "color", l->color);
            shader->SetFloat(base + "intensity", l->intensity);
            numPoint++;
        }
    }
    shader->SetInt("useDirLight", dirSet);
    shader->SetInt("numPointLights", numPoint);

    mesh->Draw();
}

void MeshRenderer::DrawGeometry(Shader& s) {
    s.SetMat4("model", owner->transform->GetModelMatrix());
    mesh->Draw();
}

// 延迟渲染几何 pass：只写材质数据，不碰光照
void MeshRenderer::DrawGBuffer(Shader& s, const Camera& cam, float aspect) {
    s.SetMat4("model", owner->transform->GetModelMatrix());
    s.SetMat4("view", cam.GetViewMatrix());
    s.SetMat4("projection", cam.GetProjectionMatrix(aspect));
    s.SetVec3("viewPos", cam.GetPosition());
    BindMaterial(s);
    mesh->Draw();
}