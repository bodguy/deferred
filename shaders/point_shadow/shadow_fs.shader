#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D diffuseTexture;
uniform samplerCube depthMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float far_plane;
uniform bool shadows;
uniform bool use_pcf;
uniform float bias;

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

float ShadowCalc(vec3 fragPos) {
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    float shadow = 0.0;

    if (use_pcf) {
        int samples = 25;
        float radius = 1.0 / 500.0;
        radius *= clamp(length(viewPos - fragPos), 0.2, 6);
        for (int i = 0; i < samples; ++i) {
            float closestDepth = texture(depthMap, fragToLight + offsets[i] * radius).r;
            closestDepth *= far_plane;
            if(currentDepth - bias > closestDepth) {
                shadow += 1.0;
            }
        }
        shadow /= float(samples);
    } else {
        float closestDepth = texture(depthMap, fragToLight).r;
        closestDepth *= far_plane;
        shadow = currentDepth - bias  > closestDepth ? 1.0 : 0.0;
    }

    return shadow;
}

void main()
{
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(0.3);
    // ambient
    vec3 ambient = 0.3 * color;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), 68.0);
    vec3 specular = spec * lightColor;
    // calculate shadow
    float shadow = shadows ? ShadowCalc(fs_in.FragPos) : 0.0;
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

    FragColor = vec4(lighting, 1.0);
}