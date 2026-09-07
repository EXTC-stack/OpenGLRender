#include "Core/Application.h"
#include "Core/Scene.h"
#include "Core/Prefabs.h"
#include "Components/Transform.h"
#include "Components/Camera.h"
#include "Components/Light.h"
#include "Components/MeshRenderer.h"
#include "Renderer/Shader.h"
#include "Renderer/Mesh.h"
#include "Renderer/Texture.h"

int main() {
    Application app;
    if (!app.Init(1280, 720, "OpenGLRender - Room")) return -1;

    auto brickHeight = Texture::LoadFromFile("assets/textures/bricks2_disp.jpg", false);
    auto litShader = std::make_shared<Shader>("shaders/forward_lit.vert", "shaders/forward_lit.frag");
    auto cubeMesh = Mesh::CreateCube();

    auto woodTex = Texture::LoadFromFile("assets/textures/container2.png", true);
    auto brickTex = Texture::LoadFromFile("assets/textures/bricks2.jpg", true);
    auto brickNorm = Texture::LoadFromFile("assets/textures/bricks2_normal.jpg", false);

    Scene scene;
    glm::vec3 wallColor = { 0.85f, 0.82f, 0.78f };
    glm::vec3 floorColor = { 0.55f, 0.45f, 0.35f };

    // ===== 房间 8×3×8，墙厚 0.2 =====
// 地板 & 天花板
    Prefabs::CreateBox(scene, cubeMesh, litShader, { 0, -0.1f, 0 }, { 8, 0.2f, 8 },
        floorColor, woodTex, nullptr, nullptr, { 4.0f, 4.0f });
    Prefabs::CreateBox(scene, cubeMesh, litShader, { 0, 3.1f, 0 }, { 8, 0.2f, 8 },
        wallColor);

    // 前墙（z=+4）
    Prefabs::CreateBox(scene, cubeMesh, litShader, { 0, 1.5f, 4 }, { 8, 3, 0.2f },
        wallColor, brickTex, brickNorm, brickHeight, { 8.0f, 3.0f });
    // 后墙（z=-4）
    Prefabs::CreateBox(scene, cubeMesh, litShader, { 0, 1.5f, -4 }, { 8, 3, 0.2f },
        wallColor, brickTex, brickNorm, brickHeight, { 8.0f, 3.0f });
    // 左墙（x=-4）
    Prefabs::CreateBox(scene, cubeMesh, litShader, { -4, 1.5f, 0 }, { 0.2f, 3, 8 },
        wallColor, brickTex, brickNorm, brickHeight, { 8.0f, 3.0f });

    // 右墙（x=+4）：留一个 2.4 宽、0.9~2.1 高的窗洞（第 2 步装玻璃）
    // 窗洞左侧段
    Prefabs::CreateBox(scene, cubeMesh, litShader, { 4, 1.5f, -2.6f }, { 0.2f, 3, 2.8f },
        wallColor, brickTex, brickNorm, brickHeight, { 2.8f, 3.0f });
    // 窗洞右侧段
    Prefabs::CreateBox(scene, cubeMesh, litShader, { 4, 1.5f, 2.6f }, { 0.2f, 3, 2.8f },
        wallColor, brickTex, brickNorm, brickHeight, { 2.8f, 3.0f });
    // 窗洞下方墙段
    Prefabs::CreateBox(scene, cubeMesh, litShader, { 4, 0.45f, 0 }, { 0.2f, 0.9f, 2.4f },
        wallColor, brickTex, brickNorm, brickHeight, { 2.4f, 0.9f });
    // 窗洞上方墙段
    Prefabs::CreateBox(scene, cubeMesh, litShader, { 4, 2.55f, 0 }, { 0.2f, 0.9f, 2.4f },
        wallColor, brickTex, brickNorm, brickHeight, { 2.4f, 0.9f });

    // 窗框（上下沿）
    Prefabs::CreateBox(scene, cubeMesh, litShader, { 4, 0.95f, 0 }, { 0.3f, 0.1f, 2.5f },
        { 0.4f, 0.3f, 0.2f }, woodTex);
    Prefabs::CreateBox(scene, cubeMesh, litShader, { 4, 2.05f, 0 }, { 0.3f, 0.1f, 2.5f },
        { 0.4f, 0.3f, 0.2f }, woodTex);

    // ===== 玻璃窗 =====
    Object* glass = Prefabs::CreateBox(scene, cubeMesh, litShader,
        { 4, 1.5f, 0 }, { 0.05f, 1.2f, 2.4f }, { 0.6f, 0.8f, 0.9f });
    auto* glassMr = glass->GetComponent<MeshRenderer>();
    glassMr->transparent = true;
    glassMr->alpha = 0.25f;
    glassMr->shininess = 128.0f;   // 玻璃高光更锐利

    // ===== 窗外景色 =====
    // 草地
    Prefabs::CreateBox(scene, cubeMesh, litShader, { 12, -0.2f, 0 }, { 16, 0.2f, 16 }, { 0.3f, 0.5f, 0.25f });
    // 三棵树（树干 + 树冠）
    auto tree = [&](float x, float z) {
        Prefabs::CreateBox(scene, cubeMesh, litShader, { x, 0.5f, z }, { 0.3f, 1.0f, 0.3f }, { 0.4f, 0.3f, 0.2f });
        Prefabs::CreateBox(scene, cubeMesh, litShader, { x, 1.5f, z }, { 1.2f, 1.2f, 1.2f }, { 0.2f, 0.5f, 0.2f });
        };
    tree(8.0f, -2.0f);
    tree(10.0f, 1.5f);
    tree(7.0f, 3.0f);
    // 远处房子剪影
    Prefabs::CreateBox(scene, cubeMesh, litShader, { 14, 1.0f, -4 }, { 3, 2, 3 }, { 0.5f, 0.4f, 0.35f });

    // ===== 家具 =====
    Prefabs::CreateTable(scene, cubeMesh, litShader, { 0, 0, -1 }, woodTex);
    Prefabs::CreateChair(scene, cubeMesh, litShader, { 0, 0, 0.3f }, 180.0f, woodTex);  // 面朝桌子
    Prefabs::CreateChair(scene, cubeMesh, litShader, { 0, 0, -2.3f }, 0.0f, woodTex);

    // 旋转木箱留作桌面摆件
    Object* cube = Prefabs::CreateCube(scene, cubeMesh, litShader, { 0.4f, 1.0f, -1.0f },
        woodTex, nullptr);
    cube->transform->scale = { 0.3f, 0.3f, 0.3f };

    // ===== 灯光 =====
    // 夕阳方向光，从窗户（+x 方向）斜射进来
    Prefabs::CreateDirectionalLight(scene, { -0.9f, -0.5f, -0.15f }, { 1.0f, 0.85f, 0.7f }, 0.9f);
    // 桌上暖色吊灯，带阴影
    Object* lamp = Prefabs::CreatePointLight(scene, { 0, 2.3f, -1 }, { 1.0f, 0.8f, 0.55f }, 4.0f);
    lamp->GetComponent<Light>()->castShadow = true;

    // ===== 相机：站在房间角落看向桌子 =====
    Object* camObj = Prefabs::CreateMainCamera(scene, app.GetWindow(), { -2.8f, 1.7f, 2.8f });
    auto* cam = camObj->GetComponent<Camera>();
    cam->yaw = -40.0f;
    cam->pitch = -14.0f;

    app.Run(scene, [&](float dt) {
        cube->transform->rotationEuler.y += 45.0f * dt;
        });
    app.Shutdown();
    return 0;
}