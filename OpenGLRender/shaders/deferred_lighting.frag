#version 450 core
out vec4 FragColor;
in vec2 TexCoords;

uniform samplerCube pointShadowMap;
uniform int usePointShadow;
uniform int pointShadowIndex;    // 哪个点光源有阴影
uniform vec3 pointShadowPos;
uniform float pointFarPlane;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform vec3 viewPos;
uniform float shininess;

uniform sampler2D shadowMap;
uniform int useShadow;
uniform mat4 lightSpaceMatrix;

struct DirLight {
    vec3 direction;
    vec3 color;
    float intensity;
};
uniform DirLight dirLight;
uniform int useDirLight;

#define MAX_POINT_LIGHTS 32   // 延迟渲染不怕灯多
struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numPointLights;

vec3 albedo;
float specStrength;

float PointShadowCalculation(vec3 fragPos) {
    vec3 fragToLight = fragPos - pointShadowPos;
    float currentDepth = length(fragToLight);

    vec3 offsets[20] = vec3[](
        vec3( 1, 1, 1), vec3( 1,-1, 1), vec3(-1,-1, 1), vec3(-1, 1, 1),
        vec3( 1, 1,-1), vec3( 1,-1,-1), vec3(-1,-1,-1), vec3(-1, 1,-1),
        vec3( 1, 1, 0), vec3( 1,-1, 0), vec3(-1,-1, 0), vec3(-1, 1, 0),
        vec3( 1, 0, 1), vec3(-1, 0, 1), vec3( 1, 0,-1), vec3(-1, 0,-1),
        vec3( 0, 1, 1), vec3( 0,-1, 1), vec3( 0,-1,-1), vec3( 0, 1,-1)
    );

    float shadow = 0.0;
    float bias = 0.15;
    float diskRadius = 0.05;
    for (int i = 0; i < 20; i++) {
        float closestDepth = texture(pointShadowMap, fragToLight + offsets[i] * diskRadius).r;
        closestDepth *= pointFarPlane;   // [0,1] 还原回距离
        shadow += (currentDepth - bias > closestDepth) ? 1.0 : 0.0;
    }
    return shadow / 20.0;
}

vec3 BlinnPhong(vec3 lightDir, vec3 lightColor, float intensity, vec3 normal, vec3 viewDir) {
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    return (diff * albedo + spec * specStrength * vec3(1.0)) * lightColor * intensity;
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0) return 0.0;

    float currentDepth = projCoords.z;
    float bias = max(0.002 * (1.0 - dot(normal, lightDir)), 0.0005);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
        }
    }
    return shadow / 9.0;
}

void main() {
    vec3 normal = texture(gNormal, TexCoords).xyz;

    // 背景像素（G-Buffer 清零的地方）直接输出背景色
    if (dot(normal, normal) < 0.01) {
        FragColor = vec4(0.1, 0.12, 0.15, 1.0);
        return;
    }

    vec3 FragPos = texture(gPosition, TexCoords).xyz;
    vec4 albedoSpec = texture(gAlbedoSpec, TexCoords);
    albedo = albedoSpec.rgb;
    specStrength = albedoSpec.a;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = 0.08 * albedo;

    if (useDirLight == 1) {
        vec3 lightDir = normalize(-dirLight.direction);
        vec3 direct = BlinnPhong(lightDir, dirLight.color, dirLight.intensity, normal, viewDir);
        float shadow = 0.0;
        if (useShadow == 1) {
            vec4 fragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
            shadow = ShadowCalculation(fragPosLightSpace, normal, lightDir);
        }
        result += (1.0 - shadow) * direct;
    }

    for (int i = 0; i < numPointLights; i++) {
        vec3 lightDir = normalize(pointLights[i].position - FragPos);
        float dist = length(pointLights[i].position - FragPos);
        float attenuation = 1.0 / (dist * dist + 1.0);
        vec3 direct = BlinnPhong(lightDir, pointLights[i].color,
                                 pointLights[i].intensity * attenuation, normal, viewDir);

        float shadow = 0.0;
        if (usePointShadow == 1 && i == pointShadowIndex)
            shadow = PointShadowCalculation(FragPos);

        result += (1.0 - shadow) * direct;
    }

    FragColor = vec4(result, 1.0);
}