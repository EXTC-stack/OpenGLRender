#include "Renderer/Framebuffer.h"
#include <glad/glad.h>
#include <iostream>

Framebuffer::~Framebuffer() {
    if (FBO) glDeleteFramebuffers(1, &FBO);
    if (depthTexture) glDeleteTextures(1, &depthTexture);
    if (depthCubemap) glDeleteTextures(1, &depthCubemap);
}

std::shared_ptr<Framebuffer> Framebuffer::CreateDepthOnly(int w, int h) {
    auto fb = std::make_shared<Framebuffer>();
    fb->width = w; fb->height = h;

    glGenFramebuffers(1, &fb->FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, fb->FBO);

    glGenTextures(1, &fb->depthTexture);
    glBindTexture(GL_TEXTURE_2D, fb->depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0,
        GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // 关键：超出阴影图范围的区域视为"不在阴影中"（边界设为白色=最远深度）
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float border[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fb->depthTexture, 0);
    glDrawBuffer(GL_NONE);   // 没有颜色附件，显式声明
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "[Framebuffer] 不完整！\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return fb;
}

void Framebuffer::Bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glViewport(0, 0, width, height);
}

void Framebuffer::Unbind(int screenW, int screenH) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screenW, screenH);
}

void Framebuffer::BindDepthTexture(unsigned int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
}

std::shared_ptr<Framebuffer> Framebuffer::CreateDepthCubemap(int size) {
    auto fb = std::make_shared<Framebuffer>();
    fb->width = fb->height = size;

    glGenTextures(1, &fb->depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, fb->depthCubemap);
    for (int i = 0; i < 6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
            size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &fb->FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, fb->FBO);
    // 关键：glFramebufferTexture 把整张 cubemap 作为分层附件挂上，
    // 几何着色器里用 gl_Layer 选择写入哪个面
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fb->depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "[Framebuffer] cubemap 不完整！\n";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return fb;
}

void Framebuffer::BindDepthCubemap(unsigned int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
}