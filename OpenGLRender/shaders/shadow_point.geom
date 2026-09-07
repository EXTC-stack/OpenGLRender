#version 450 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;   // 6 面 × 3 顶点

uniform mat4 lightSpaceMatrices[6];

out vec4 FragPos;

void main() {
    for (int face = 0; face < 6; face++) {
        gl_Layer = face;          // 这个图元写到 cubemap 的第 face 层
        for (int i = 0; i < 3; i++) {
            FragPos = gl_in[i].gl_Position;   // 世界坐标，传给 frag 算距离
            gl_Position = lightSpaceMatrices[face] * FragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}