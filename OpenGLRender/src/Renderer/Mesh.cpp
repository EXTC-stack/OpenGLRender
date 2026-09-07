#include "Renderer/Mesh.h"
#include <glad/glad.h>
#include <glm/glm.hpp>

Mesh::Mesh(const std::vector<float>& vertices) {
    vertexCount = (int)vertices.size() / 14;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);   // position
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);   // normal
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);   // uv
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);   // tangent
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    glEnableVertexAttribArray(4);   // bitangent

    glBindVertexArray(0);
}

Mesh::Mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices)
    : Mesh(vertices) {
    indexCount = (int)indices.size();

    glBindVertexArray(VAO);
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
}

void Mesh::Draw() const {
    glBindVertexArray(VAO);
    if (indexCount > 0)
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    else
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glBindVertexArray(0);
}

std::shared_ptr<Mesh> Mesh::CreateCube() {
    std::vector<float> v;

    // 每个面 4 个顶点，UV 固定 (0,0)(1,0)(1,1)(0,1)，
    // 切线 T = UV 中 u 增大的方向，B = N × T
    auto addFace = [&](glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3,
        glm::vec3 n, glm::vec3 t) {
            glm::vec3 b = glm::cross(n, t);
            glm::vec3 ps[4] = { p0, p1, p2, p3 };
            glm::vec2 uvs[4] = { {0,0}, {1,0}, {1,1}, {0,1} };
            for (int i = 0; i < 4; i++) {
                v.insert(v.end(), { ps[i].x, ps[i].y, ps[i].z,
                                    n.x, n.y, n.z,
                                    uvs[i].x, uvs[i].y,
                                    t.x, t.y, t.z,
                                    b.x, b.y, b.z });
            }
        };

    addFace({ -0.5f,-0.5f, 0.5f }, { 0.5f,-0.5f, 0.5f }, { 0.5f, 0.5f, 0.5f }, { -0.5f, 0.5f, 0.5f }, { 0, 0, 1 }, { 1, 0, 0 });  // 前
    addFace({ 0.5f,-0.5f,-0.5f }, { -0.5f,-0.5f,-0.5f }, { -0.5f, 0.5f,-0.5f }, { 0.5f, 0.5f,-0.5f }, { 0, 0,-1 }, { -1, 0, 0 }); // 后
    addFace({ -0.5f, 0.5f, 0.5f }, { 0.5f, 0.5f, 0.5f }, { 0.5f, 0.5f,-0.5f }, { -0.5f, 0.5f,-0.5f }, { 0, 1, 0 }, { 1, 0, 0 });  // 上
    addFace({ -0.5f,-0.5f,-0.5f }, { 0.5f,-0.5f,-0.5f }, { 0.5f,-0.5f, 0.5f }, { -0.5f,-0.5f, 0.5f }, { 0,-1, 0 }, { 1, 0, 0 });  // 下
    addFace({ 0.5f,-0.5f, 0.5f }, { 0.5f,-0.5f,-0.5f }, { 0.5f, 0.5f,-0.5f }, { 0.5f, 0.5f, 0.5f }, { 1, 0, 0 }, { 0, 0,-1 });  // 右
    addFace({ -0.5f,-0.5f,-0.5f }, { -0.5f,-0.5f, 0.5f }, { -0.5f, 0.5f, 0.5f }, { -0.5f, 0.5f,-0.5f }, { -1, 0, 0 }, { 0, 0, 1 }); // 左

    std::vector<unsigned int> idx;
    for (unsigned int f = 0; f < 6; f++) {
        unsigned int base = f * 4;
        idx.insert(idx.end(), { base, base + 1, base + 2,  base, base + 2, base + 3 });
    }
    return std::make_shared<Mesh>(v, idx);
}

std::shared_ptr<Mesh> Mesh::CreatePlane(float width, float height, float uvTiling) {
    float w = width * 0.5f, h = height * 0.5f;
    float t = uvTiling;

    // 面朝 +z，法线 (0,0,1)，切线 (1,0,0)，副切线 (0,1,0)
    std::vector<float> v = {
        // pos                  normal     uv        tangent    bitangent
        -w, -h, 0.0f,  0, 0, 1,  0, 0,     1, 0, 0,  0, 1, 0,
         w, -h, 0.0f,  0, 0, 1,  t, 0,     1, 0, 0,  0, 1, 0,
         w,  h, 0.0f,  0, 0, 1,  t, t,     1, 0, 0,  0, 1, 0,
        -w,  h, 0.0f,  0, 0, 1,  0, t,     1, 0, 0,  0, 1, 0,
    };
    std::vector<unsigned int> idx = { 0, 1, 2,  0, 2, 3 };
    return std::make_shared<Mesh>(v, idx);
}