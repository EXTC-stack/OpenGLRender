#include "Renderer/GBuffer.h"
#include <glad/glad.h>
#include <iostream>

GBuffer::~GBuffer() {
    if (FBO) glDeleteFramebuffers(1, &FBO);
    unsigned int texs[3] = { gPosition, gNormal, gAlbedoSpec };
    glDeleteTextures(3, texs);
    if (depthRBO) glDeleteRenderbuffers(1, &depthRBO);
}

std::unique_ptr<GBuffer> GBuffer::Create(int w, int h) {
    auto g = std::unique_ptr<GBuffer>(new GBuffer());
    g->width = w; g->height = h;

    glGenFramebuffers(1, &g->FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, g->FBO);

    auto makeAttachment = [&](unsigned int& tex, GLint internal, GLenum format,
        GLenum type, int slot) {
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexImage2D(GL_TEXTURE_2D, 0, internal, w, h, 0, format, type, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + slot, GL_TEXTURE_2D, tex, 0);
        };

    makeAttachment(g->gPosition, GL_RGBA16F, GL_RGBA, GL_FLOAT, 0);   // 世界坐标，要精度
    makeAttachment(g->gNormal, GL_RGBA16F, GL_RGBA, GL_FLOAT, 1);     // 世界法线，要精度
    makeAttachment(g->gAlbedoSpec, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 2); // rgb=颜色 a=高光强度

    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);   // MRT 关键调用

    glGenRenderbuffers(1, &g->depthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, g->depthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g->depthRBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "[GBuffer] 不完整！\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return g;
}

void GBuffer::Bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glViewport(0, 0, width, height);
}

void GBuffer::Unbind(int screenW, int screenH) const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screenW, screenH);
}

void GBuffer::BindTextures(unsigned int startUnit) const {
    glActiveTexture(GL_TEXTURE0 + startUnit);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE0 + startUnit + 1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE0 + startUnit + 2);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
}

void GBuffer::DrawQuad() {
    static unsigned int quadVAO = 0, quadVBO = 0;
    if (!quadVAO) {
        float q[] = {   // pos(2) + uv(2)，NDC 坐标
            -1, -1, 0, 0,   1, -1, 1, 0,   1, 1, 1, 1,
            -1, -1, 0, 0,   1,  1, 1, 1,  -1, 1, 0, 1,
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(q), q, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}