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
    sampler2D normal;
    bool useNormal;
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

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 WorldViewPos;

    vec3 TangentLightPos[NR_POINT_LIGHTS];
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform samplerCube depthMap[NR_POINT_LIGHTS];

uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform Material material;
uniform float far_plane;

vec3 CalcPointLight(PointLight light, vec3 lightPos, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow) {
    vec3 lightDir = normalize(lightPos - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    // attenuation
    float distance = length(lightPos - fragPos);
    float attenuation = 1.0 / (1.0 + clamp(light.attenuation, 0.0, 1.0) * pow(distance, 2));

    // combine results
    vec3 ambient = light.color * vec3(texture(material.diffuse, fs_in.TexCoords)) * light.intensity * attenuation;
    vec3 diffuse = ambient * diff; // intentional for the sake of performance
    vec3 specular = light.color * spec * vec3(texture(material.specular, fs_in.TexCoords)) * attenuation;
    return (ambient + (1.0 - shadow) * diffuse + specular);
}

// everything in world space
float CalculateShadow(vec3 fragPos, vec3 viewPos, int idx) {
    vec3 fragToLight = fragPos - pointLights[idx].position;
    float currentDepth = length(fragToLight);
    float shadow = 0.0;
    float shadowStrength = clamp(pointLights[idx].shadowStrength, 0.0, 1.0);

    if (pointLights[idx].castTranslucentShadow) {
        int samples = 25;
        float radius = pointLights[idx].shadowFilterSharpen * clamp(length(viewPos - fragPos), 0.2, 6);
        for (int i = 0; i < samples; ++i) {
            float closestDepth = texture(depthMap[idx], fragToLight + offsets[i] * radius).r;
            closestDepth *= far_plane;
            if(currentDepth - pointLights[idx].shadowBias > closestDepth) {
                shadow += shadowStrength;
            }
        }
        shadow /= float(samples);
    } else {
        float closestDepth = texture(depthMap[idx], fragToLight).r;
        closestDepth *= far_plane;
        shadow = currentDepth - pointLights[idx].shadowBias > closestDepth ? shadowStrength : 0.0;
    }

    return shadow;
}

void main() {
    vec3 normal = vec3(0.0);
    if (material.useNormal) {
        normal = texture(material.normal, fs_in.TexCoords).rgb;
        normal = normalize(normal * 2.0 - 1.0); // transform normal vector to range [-1, 1]
    } else {
        normal = normalize(fs_in.Normal);
    }
    vec3 viewDir = normalize(
        material.useNormal ? (fs_in.TangentViewPos - fs_in.TangentFragPos) : (fs_in.WorldViewPos - fs_in.FragPos)
    );

    vec3 result = vec3(0.0);
    float shadow = 0.0;
    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
        if (pointLights[i].castShadow) {
            shadow += CalculateShadow(fs_in.FragPos, fs_in.WorldViewPos, i);
        }
        result += CalcPointLight(pointLights[i],
                        material.useNormal ? fs_in.TangentLightPos[i] : pointLights[i].position,
                        normal, material.useNormal ? fs_in.TangentFragPos : fs_in.FragPos, viewDir, shadow);
    }

    FragColor = vec4(result, 1.0);
}