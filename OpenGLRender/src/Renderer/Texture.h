#pragma once
#include <string>
#include <memory>

class Texture {
public:
    unsigned int ID = 0;
    int width = 0, height = 0, channels = 0;

    ~Texture();

    // srgb：漫反射/颜色贴图传 true，高光/法线/高度等数据贴图传 false
    static std::shared_ptr<Texture> LoadFromFile(const std::string& path, bool srgb = true);

    void Bind(unsigned int unit) const;
};