# OpenGLRender

一个从零手写的 **C++20 / OpenGL 4.5** 渲染器,采用 **GameObject/Component(对象-组件)** 架构,同时实现**延迟渲染**与**前向渲染**两条管线,并支持**方向光 + 点光源阴影**。项目在 Visual Studio 2022 中构建,内置一个小房间场景作为功能演示。

# 视频链接
https://youtu.be/-iI0tppRkTs

## 技术栈

| 组件        | 用途                       |
| ----------- | -------------------------- |
| OpenGL 4.5  | 渲染 API(核心剖面 Core Profile) |
| GLFW 3.5    | 窗口、输入、上下文          |
| GLAD        | OpenGL 函数加载器           |
| GLM         | 数学库(矩阵/向量)          |
| stb_image   | 纹理加载                    |
| ImGui       | 已接入工程(尚未在 UI 中使用) |

编译标准:`C++20`(`stdcpp20`),工具集 `v143`(VS 2022)。

## 目录结构

```
OpenGLRender/
├─ src/
│  ├─ Core/          Application / Scene / Object / Component / Prefabs
│  ├─ Components/    Transform / Camera / CameraController / Light / MeshRenderer
│  ├─ Renderer/      Mesh / Shader / Texture / Framebuffer / GBuffer
│  └─ Utils/         stb_impl.cpp
├─ shaders/          11 个 GLSL 着色器
├─ assets/textures/  材质贴图
└─ third_party/      GLFW / GLAD / GLM / ImGui / stb_image
```

### 对象-组件模型
- `Object` 是一棵空的实体,自带一个 `Transform` 组件,并用 `std::unique_ptr<Component>` 持有其它组件。
- `Component` 是基类,提供 `Start()` / `Update(dt)` 虚接口;通过 `Object::AddComponent<T>()` 挂载、`GetComponent<T>()` 取回。
- `Scene` 用 `std::vector<std::unique_ptr<Object>>` 管理所有对象,每帧先 `Update(dt)`,再 `Render()`。

## 已实现的渲染特性

- **双渲染管线**:`Scene::useDeferred` 切换延迟路径(默认开启)与前向路径(保留作对比)。
- **延迟渲染(Deferred Rendering)**:
  - **G-Buffer** 三张 MRT:
    - `gPosition` = 世界坐标(`GL_RGBA16F`)
    - `gNormal`   = 世界法线(`GL_RGBA16F`)
    - `gAlbedoSpec` = 反照率(RGB)+ 镜面强度(A)(`GL_RGBA`)
  - 深度用 **Renderbuffer** `GL_DEPTH_COMPONENT` 挂载。
  - 光照阶段是全屏 Quad,一次读取 `gPosition/gNormal/gAlbedoSpec` 计算 **Blinn-Phong**。
  - 光照后通过 `glBlitFramebuffer` 把 G-Buffer 深度**回拷**到默认帧缓冲,保证透明物体正确遮挡。
- **光照模型**:Blinn-Phong(半程向量高光);支持 1 个方向光 + 最多 **32 个点光源**(延迟);前向路径点光上限为 8。点光衰减为 `1 / (d² + 1)`。
- **方向光阴影**:正交投影(`glOrtho`)+ 3×3 **PCF** 过滤,深度图用 `GL_CLAMP_TO_BORDER`,边界外视为无阴影。
- **点光源阴影(立方体阴影映射)**:6 面 cubemap 深度纹理,用**几何着色器**按 `gl_Layer` 一次写入 6 个面;采样时 20 个方向偏移做软阴影。
- **视差遮蔽映射(POM)**:基于高度图在切线空间沿视线步进,找出实际命中深度层,并进行线性插值消除层状条纹。
- **材质系统**:每份材质可绑定 diffuse / specular / normal / height 四张贴图,外加 `shininess`(高光系数)、`uvTiling`(UV 平铺)、`alpha` 与透明标志;法线/高度/高光贴图以数据贴图方式加载(不开启 sRGB)。
- **透明物体**:跳过 G-Buffer,走独立的前向 pass;开启 Alpha 混合、关闭深度写入(`glDepthMask(GL_FALSE)`),仅做深度测试。
- **自由相机**:右键拖拽视角(鼠标锁定),`WASD + QE` 移动,`Shift` 加速。

## 着色器列表

| 着色器                          | 用途                                    |
| ------------------------------- | --------------------------------------- |
| `forward_lit.vert/.frag`        | 前向光照(Blinn-Phong + POM + 阴影)      |
| `gbuffer.vert/.frag`            | 延迟几何 pass,写三张 G-Buffer           |
| `deferred_lighting.vert/.frag`  | 延迟光照 pass,全屏 Quad,读 G-Buffer     |
| `shadow_depth.vert/.frag`       | 方向光阴影深度图                        |
| `shadow_point.vert/.geom/.frag` | 点光源 cubemap 阴影(几何着色器多层写)  |

## 构建与运行

### 环境
- Windows 10 / 11
- Visual Studio 2022(需安装 **C++ 桌面开发** 工作负载;工具集 `v143`)

### 步骤
1. 用 Visual Studio 打开 `OpenGLRender.slnx`。
2. 选择 `Debug | x64`(已配置 GLFW 库路径与 `glfw3.lib`,并定义 `GLFW_STATIC` 做静态链接)。
3. 编译并运行(`Ctrl+F5`)。

> **工作目录说明**:着色器与纹理通过 `shaders/...`、`assets/textures/...` 相对路径读取,因此 **exe 必须在工程目录(即与 `OpenGLRender.vcxproj` 同层的目录)下运行**。在 VS 中调试时默认工作目录就是该目录,可直接运行;若手动启动 exe,请先把工作目录切到该层。

## 操作说明

| 按键/操作                    | 功能                 |
| ---------------------------- | -------------------- |
| 按住**鼠标右键**并拖动       | 旋转视角(相机)      |
| `W / A / S / D`              | 前后左右移动         |
| `Q / E`                      | 下 / 上              |
| 按住 `Shift`                 | 加速移动             |
| `Esc`                        | 退出程序             |

## 演示场景(`main.cpp`)

在一个 **8×3×8** 的房间内:
- 地板、天花板、四面砖墙(带窗洞)、窗框与**半透明玻璃窗**(alpha=0.25,shininess=128)。
- 窗外草地、三棵树与远处房子剪影。
- 木桌、两把椅子与桌上一枚随帧旋转的木箱。
- 灯光:一束从窗户斜射进来的**夕阳方向光**(带阴影)+ 桌上**暖色点光吊灯**(带 cubemap 阴影)。

## 说明与已知限制

- `third_party/ImGui` 已经随工程编译,但代码中**尚未真正接入 UI**。
- 点光源阴影**目前仅支持 1 个**(取第一个开启 `castShadow` 的点光源)。
- Debug 链接 GLFW 静态库 `glfw3.lib`(发布版 CRT)`可能与 Debug 运行库(`/MDd`)有轻微 CRT 冲突,链接时会出现 `LNK4098` 警告,不影响运行。
