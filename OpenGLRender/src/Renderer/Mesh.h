#pragma once
#include <vector>
#include <memory>

class Mesh {
public:
    unsigned int VAO = 0, VBO = 0, EBO = 0;
    int vertexCount = 0;
    int indexCount = 0;

    // 顶点格式：position(3 floats) + normal(3 floats)
    explicit Mesh(const std::vector<float>& vertices);
    Mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices);
    ~Mesh();
    void Draw() const;

    static std::shared_ptr<Mesh> CreateCube();
    static std::shared_ptr<Mesh> CreatePlane(float width = 1.0f, float height = 1.0f, float uvTiling = 1.0f);

};