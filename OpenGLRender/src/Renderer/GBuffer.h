#pragma once
#include <memory>

class GBuffer {
public:
    unsigned int FBO = 0;
    int width = 0, height = 0;
    unsigned int gPosition = 0, gNormal = 0, gAlbedoSpec = 0;  // 三张 MRT 纹理
    unsigned int depthRBO = 0;                                  // 深度用 renderbuffer

    ~GBuffer();

    static std::unique_ptr<GBuffer> Create(int w, int h);

    void Bind() const;
    void Unbind(int screenW, int screenH) const;
    void BindTextures(unsigned int startUnit = 0) const;  // 光照 pass 采样用

    static void DrawQuad();   // 全屏四边形
};