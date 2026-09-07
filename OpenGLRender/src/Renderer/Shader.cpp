#include "Renderer/Shader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

void Shader::SetVec2(const std::string& n, const glm::vec2& v) const {
    glUniform2fv(glGetUniformLocation(ID, n.c_str()), 1, glm::value_ptr(v));
}

static std::string ReadFile(const char* path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "[Shader] Can't Open The File: " << path << "\n";
        return "";
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static unsigned int CompileStage(unsigned int type, const std::string& src, const char* path) {
    unsigned int s = glCreateShader(type);
    const char* c = src.c_str();
    glShaderSource(s, 1, &c, nullptr);
    glCompileShader(s);
    int ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetShaderInfoLog(s, 2048, nullptr, log);
        std::cerr << "[Shader] Build Faild " << path << ":\n" << log << "\n";
    }
    return s;
}

Shader::Shader(const char* vertPath, const char* fragPath, const char* geomPath) {
    unsigned int vs = CompileStage(GL_VERTEX_SHADER, ReadFile(vertPath), vertPath);
    unsigned int fs = CompileStage(GL_FRAGMENT_SHADER, ReadFile(fragPath), fragPath);

    ID = glCreateProgram();
    glAttachShader(ID, vs);
    glAttachShader(ID, fs);

    unsigned int gs = 0;
    if (geomPath) {   // 有几何着色器才编译挂载
        gs = CompileStage(GL_GEOMETRY_SHADER, ReadFile(geomPath), geomPath);
        glAttachShader(ID, gs);
    }

    glLinkProgram(ID);

    int ok;
    glGetProgramiv(ID, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetProgramInfoLog(ID, 2048, nullptr, log);
        std::cerr << "[Shader] 链接失败:\n" << log << "\n";
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (gs) glDeleteShader(gs);
}

void Shader::Use() const { glUseProgram(ID); }

void Shader::SetInt(const std::string& n, int v) const {
    glUniform1i(glGetUniformLocation(ID, n.c_str()), v);
}
void Shader::SetFloat(const std::string& n, float v) const {
    glUniform1f(glGetUniformLocation(ID, n.c_str()), v);
}
void Shader::SetVec3(const std::string& n, const glm::vec3& v) const {
    glUniform3fv(glGetUniformLocation(ID, n.c_str()), 1, glm::value_ptr(v));
}
void Shader::SetMat4(const std::string& n, const glm::mat4& v) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, n.c_str()), 1, GL_FALSE, glm::value_ptr(v));
}