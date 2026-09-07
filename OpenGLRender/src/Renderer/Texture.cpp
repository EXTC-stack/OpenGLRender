#include "Renderer/Texture.h"
#include <glad/glad.h>
#include "stb_image.h"
#include <iostream>

Texture::~Texture() {
    if (ID) glDeleteTextures(1, &ID);
}

std::shared_ptr<Texture> Texture::LoadFromFile(const std::string& path, bool srgb) {
    stbi_set_flip_vertically_on_load(true);

    auto tex = std::make_shared<Texture>();
    unsigned char* data = stbi_load(path.c_str(), &tex->width, &tex->height, &tex->channels, 0);
    if (!data) {
        std::cerr << "[Texture] Load Failed: " << path << "\n";
        return nullptr;   // 返回空，渲染时回退到纯色
    }

    GLenum internal, format;
    if (tex->channels == 1) { internal = GL_RED;                    format = GL_RED; }
    else if (tex->channels == 3) { internal = srgb ? GL_SRGB : GL_RGB;   format = GL_RGB; }
    else { internal = srgb ? GL_SRGB_ALPHA : GL_RGBA; format = GL_RGBA; }

    glGenTextures(1, &tex->ID);
    glBindTexture(GL_TEXTURE_2D, tex->ID);
    glTexImage2D(GL_TEXTURE_2D, 0, internal, tex->width, tex->height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    std::cout << "[Texture] Load Success: " << path << " (" << tex->width << "x" << tex->height << ")\n";
    return tex;
}

void Texture::Bind(unsigned int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, ID);
}