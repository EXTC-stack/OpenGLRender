#version 450 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;
in vec3 TangentW;
in vec3 BitangentW;

uniform vec2 uvTiling;
uniform float objectAlpha;

uniform vec3 viewPos;
uniform vec3 objectColor;
uniform float shininess;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform int useDiffuseMap;
uniform int useSpecularMap;

uniform sampler2D normalMap;
uniform int useNormalMap;

uniform sampler2D heightMap;
uniform int useHeightMap;
uniform float heightScale;

uniform sampler2D shadowMap;
uniform int useShadow;

struct DirLight {
    vec3 direction;
    vec3 color;
    float intensity;
};
uniform DirLight dirLight;
uniform int useDirLight;

#define MAX_POINT_LIGHTS 8
struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numPointLights;

vec3 albedo;
float specStrength;

vec3 BlinnPhong(vec3 lightDir, vec3 lightColor, float intensity, vec3 normal, vec3 viewDir) {
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    return (diff * albedo + spec * specStrength * vec3(1.0)) * lightColor * intensity;
}

// 视差遮蔽映射（POM）：在切线空间沿视线步进，找出视线实际"打到"的深度层
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDirTangent) {
    // 视角越垂直层数越少，越掠射层数越多
    const float minLayers = 8.0, maxLayers = 32.0;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0, 0, 1), viewDirTangent)));
    float layerDepth = 1.0 / numLayers;
    vec2 deltaTexCoords = viewDirTangent.xy / viewDirTangent.z * heightScale / numLayers;

    vec2 currentTexCoords = texCoords;
    float currentDepth = texture(heightMap, currentTexCoords).r;
    float currentLayerDepth = 0.0;

    while (currentLayerDepth < currentDepth) {
        currentTexCoords -= deltaTexCoords;
        currentDepth = texture(heightMap, currentTexCoords).r;
        currentLayerDepth += layerDepth;
    }

    // 与上一层做线性插值，消除层状条纹
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
    float afterDepth = currentDepth - currentLayerDepth;
    float beforeDepth = texture(heightMap, prevTexCoords).r - currentLayerDepth + layerDepth;
    float weight = afterDepth / (afterDepth - beforeDepth);
    return mix(currentTexCoords, prevTexCoords, weight);
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
    vec3 geomNormal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    mat3 TBN = mat3(normalize(TangentW), normalize(BitangentW), geomNormal);

    // 1. 视差：先偏移 UV（在法线贴图之前，用偏移后的 UV 采样所有贴图）
    vec2 texCoords = TexCoords * uvTiling;
    if (useHeightMap == 1) {
        vec3 viewDirTangent = transpose(TBN) * viewDir;  // 正交矩阵的逆 = 转置
        texCoords = ParallaxMapping(TexCoords, viewDirTangent);
    }

    // 2. 采样（法线/高度/高光贴图都不能开 sRGB）
    albedo = (useDiffuseMap == 1) ? texture(diffuseMap, texCoords).rgb : objectColor;
    specStrength = (useSpecularMap == 1) ? texture(specularMap, texCoords).r : 0.6;

    // 3. 法线贴图：切线空间 → 世界空间
    vec3 normal = geomNormal;
    if (useNormalMap == 1) {
        vec3 n = texture(normalMap, texCoords).rgb * 2.0 - 1.0;  // [0,1] → [-1,1]
        normal = normalize(TBN * n);
    }

    vec3 result = 0.08 * albedo;

    if (useDirLight == 1) {
        vec3 lightDir = normalize(-dirLight.direction);
        vec3 direct = BlinnPhong(lightDir, dirLight.color, dirLight.intensity, normal, viewDir);
        float shadow = 0.0;
        if (useShadow == 1)
            shadow = ShadowCalculation(FragPosLightSpace, normal, lightDir);
        result += (1.0 - shadow) * direct;
    }

    for (int i = 0; i < numPointLights; i++) {
        vec3 lightDir = normalize(pointLights[i].position - FragPos);
        float dist = length(pointLights[i].position - FragPos);
        float attenuation = 1.0 / (dist * dist + 1.0);
        result += BlinnPhong(lightDir, pointLights[i].color,
                             pointLights[i].intensity * attenuation, normal, viewDir);
    }

    FragColor = vec4(result, objectAlpha);
}