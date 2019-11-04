#version 330 core
out vec4 FragColor;

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
    float shininess;
};

vec3 offsets[25] = vec3[] (
    vec3( 0,  0,  0), vec3( 0,  1,  1), vec3( 0, -1,  1),
    vec3( 0, -1, -1), vec3( 0,  1, -1), vec3( 0,  0, -1),
    vec3( 0,  0,  1), vec3( 0, -1,  0), vec3( 0,  1,  0),

    vec3( 1,  0,  0)                  , vec3( 1, -1,  1),
    vec3( 1, -1, -1), vec3( 1,  1, -1), vec3( 1,  0, -1),
    vec3( 1,  0,  1), vec3( 1, -1,  0), vec3( 1,  1,  0),

    vec3( -1, 0,  0), vec3(-1,  1,  1), vec3(-1, -1,  1),
    vec3(-1,  1, -1), vec3(-1,  0, -1),
    vec3(-1,  0,  1), vec3(-1, -1,  0), vec3(-1,  1,  0)
);

vec3 sampleOffsetDirections[20] = vec3[] (
    vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
    vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
    vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
    vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
    vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform samplerCube depthMap[NR_POINT_LIGHTS];

uniform vec3 viewPos;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform Material material;

uniform float far_plane;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow) {
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (1.0 + clamp(light.attenuation, 0, 1) * pow(distance, 2));

    // combine results
    vec3 ambient = light.color * vec3(texture(material.diffuse, fs_in.TexCoords)) * light.intensity * attenuation;
    vec3 diffuse = light.color * diff * vec3(texture(material.diffuse, fs_in.TexCoords)) * light.intensity * attenuation;
    vec3 specular = light.color * spec * vec3(texture(material.specular, fs_in.TexCoords)) * attenuation;
    return (ambient + (1.0 - shadow) * (diffuse + specular));
}

float CalculateShadow(vec3 fragPos, int idx) {
    vec3 fragToLight = fragPos - pointLights[idx].position;
    float currentDepth = length(fragToLight);
    float shadow = 0.0;

    if (pointLights[idx].castTranslucentShadow) {
        int samples = 25;
        float radius = pointLights[idx].shadowFilterSharpen * clamp(length(viewPos - fragPos), 0.2, 6);
        for (int i = 0; i < samples; ++i) {
            float closestDepth = texture(depthMap[idx], fragToLight + offsets[i] * radius).r;
            closestDepth *= far_plane;
            if(currentDepth - pointLights[idx].shadowBias > closestDepth) {
                shadow += clamp(pointLights[idx].shadowStrength, 0, 1);
            }
        }
        shadow /= float(samples);
    } else {
        float closestDepth = texture(depthMap[idx], fragToLight).r;
        closestDepth *= far_plane;
        shadow = currentDepth - pointLights[idx].shadowBias  > closestDepth ? clamp(pointLights[idx].shadowStrength, 0, 1) : 0.0;
    }

    return shadow;
}

void main() {
    vec3 normal = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    vec3 result = vec3(0.0);
    float shadow = 0.0;
    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
        if (pointLights[i].castShadow) {
            shadow += CalculateShadow(fs_in.FragPos, i);
        }
        result += CalcPointLight(pointLights[i], normal, fs_in.FragPos, viewDir, shadow);
    }

    FragColor = vec4(result, 1.0);
}