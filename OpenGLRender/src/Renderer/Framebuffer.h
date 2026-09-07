#pragma once
#include <memory>

class Framebuffer {
public:
    unsigned int FBO = 0;
    int width = 0, height = 0;
    unsigned int depthTexture = 0;
    unsigned int depthCubemap = 0;    // 新增：cubemap 深度纹理

         // 新增

    ~Framebuffer();

    // 阴影图：只有深度附件
    static std::shared_ptr<Framebuffer> CreateDepthOnly(int w, int h);

    void Bind() const;
    static void Unbind(int screenW, int screenH);  // 回到默认帧缓冲并恢复视口
    void BindDepthTexture(unsigned int unit) const;

    static std::shared_ptr<Framebuffer> CreateDepthCubemap(int size);  // 新增
    void BindDepthCubemap(unsigned int unit) const;
};
