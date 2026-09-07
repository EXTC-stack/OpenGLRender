#version 450 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 TangentW;
in vec3 BitangentW;

uniform vec2 uvTiling;

uniform vec3 viewPos;
uniform vec3 objectColor;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform int useDiffuseMap;
uniform int useSpecularMap;
uniform sampler2D normalMap;
uniform int useNormalMap;
uniform sampler2D heightMap;
uniform int useHeightMap;
uniform float heightScale;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDirTangent) {
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

    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
    float afterDepth = currentDepth - currentLayerDepth;
    float beforeDepth = texture(heightMap, prevTexCoords).r - currentLayerDepth + layerDepth;
    float weight = afterDepth / (afterDepth - beforeDepth);
    return mix(currentTexCoords, prevTexCoords, weight);
}

void main() {
    vec3 geomNormal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    mat3 TBN = mat3(normalize(TangentW), normalize(BitangentW), geomNormal);

    vec2 texCoords = TexCoords * uvTiling;
    if (useHeightMap == 1) {
        vec3 viewDirTangent = transpose(TBN) * viewDir;
        texCoords = ParallaxMapping(TexCoords, viewDirTangent);
    }

    vec3 albedo = (useDiffuseMap == 1) ? texture(diffuseMap, texCoords).rgb : objectColor;
    float specStrength = (useSpecularMap == 1) ? texture(specularMap, texCoords).r : 0.6;

    vec3 normal = geomNormal;
    if (useNormalMap == 1)
        normal = normalize(TBN * (texture(normalMap, texCoords).rgb * 2.0 - 1.0));

    gPosition = FragPos;
    gNormal = normal;
    gAlbedoSpec = vec4(albedo, specStrength);
}