#pragma once
#include <string>
#include <glm/glm.hpp>

class Shader {
public:
    unsigned int ID = 0;

    Shader(const char* vertPath, const char* fragPath, const char* geomPath = nullptr);
    void Use() const;

    void SetInt(const std::string& name, int v) const;
    void SetFloat(const std::string& name, float v) const;
    void SetVec3(const std::string& name, const glm::vec3& v) const;
    void SetMat4(const std::string& name, const glm::mat4& v) const;
    void SetVec2(const std::string& n, const glm::vec2& v) const;
};