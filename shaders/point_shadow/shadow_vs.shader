#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

#define NR_POINT_LIGHTS 3

struct PointLight {
    vec3 position;
    vec3 color;
    float attenuation;
    float shadowBias;
    float shadowFilterSharpen;
    float shadowStrength;
    float intensity;
    bool castShadow;
    bool castTranslucentShadow;
};

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D normal;
    bool useNormal;
    float shininess;
};

out vec2 TexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 WorldViewPos;

    vec3 TangentLightPos[NR_POINT_LIGHTS];
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 viewPos;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform Material material;

void main() {
    mat3 normalMatrix = transpose(inverse(mat3(model)));

    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.Normal = normalMatrix * aNormal;
    vs_out.TexCoords = aTexCoords;
    vs_out.WorldViewPos = vec3(model * vec4(viewPos, 1.0));

    if (material.useNormal) {
        vec3 T = normalize(normalMatrix * aTangent);
        vec3 N = normalize(normalMatrix * aNormal);
        T = normalize(T - dot(T, N) * N);
        vec3 B = cross(N, T);

        mat3 TBN = transpose(mat3(T, B, N));
        for(int i = 0; i < NR_POINT_LIGHTS; i++) {
            vs_out.TangentLightPos[i] = TBN * pointLights[i].position;
        }
        vs_out.TangentViewPos = TBN * viewPos;
        vs_out.TangentFragPos = TBN * vs_out.FragPos;
    }

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}