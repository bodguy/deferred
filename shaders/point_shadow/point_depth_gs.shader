#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadowMatrices[6];

out vec4 FragPos;

void main() {
    // 6 * 3 = 18 vertices
    for (int face = 0; face < 6; ++face) {
        gl_Layer = face; // specify output cubemap face with built-in variable.
        for (int i = 0; i < 3; ++i) {
            FragPos = gl_in[i].gl_Position; // for using on fragment shader
            gl_Position = shadowMatrices[face] * FragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}