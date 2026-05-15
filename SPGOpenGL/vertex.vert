#version 400

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelMatrix;
uniform mat4 normalMatrix;
uniform float time;
uniform float waterLevel;
uniform int isWater;
uniform int isGrass;
uniform float shellHeight;

out vec3 normal;
out vec3 pos;
out vec3 localPos;
out vec2 texCoord;
out vec3 localPosUndisplaced; // <-- VARIABILĂ NOUĂ

const float TWO_PI = 6.28318530718;

// Modificăm parametrul: origP va fi poziția inițială fixă a vârfului
vec3 gerstnerWave(vec2 direction, float amplitude, float wavelength, float speed, float steepness,
                  vec3 origP, inout vec3 tangent, inout vec3 binormal)
{
    vec2 d = normalize(direction);
    float k = TWO_PI / wavelength;
    float phase = k * dot(d, origP.xz) + speed * time; // <-- Folosește origP static
    float s = sin(phase);
    float c = cos(phase);
    float q = steepness / (k * amplitude * 4.0);
    float qwa = q * amplitude * k;

    tangent += vec3(-d.x * d.x * qwa * s, d.x * amplitude * k * c, -d.x * d.y * qwa * s);
    binormal += vec3(-d.x * d.y * qwa * s, d.y * amplitude * k * c, -d.y * d.y * qwa * s);

    float horizontalChop = 0.18;
    return vec3(d.x * q * amplitude * c * horizontalChop, amplitude * s, d.y * q * amplitude * c * horizontalChop);
}

void main()
{
    vec3 p = vPos;
    vec3 vertexNormal = vNormal;
    vec3 origP = p; 

    if (isWater == 1) {
        p.y = waterLevel;
        origP.y = waterLevel; 
        
        vec3 tangent = vec3(1.0, 0.0, 0.0);
        vec3 binormal = vec3(0.0, 0.0, 1.0);

        // Trimitem origP la toate valurile pentru a preveni feedback-ul distructiv
        p += gerstnerWave(vec2(1.0, 0.35), 1.6, 120.0, 0.90, 0.62, origP, tangent, binormal);
        p += gerstnerWave(vec2(0.35, 1.0), 0.9, 54.0, 1.55, 0.48, origP, tangent, binormal);
        p += gerstnerWave(vec2(-0.8, 0.6), 0.55, 28.0, 2.10, 0.34, origP, tangent, binormal);
        p += gerstnerWave(vec2(0.65, -0.75), 0.35, 18.0, 2.75, 0.24, origP, tangent, binormal);

        vertexNormal = normalize(cross(binormal, tangent));
    }

    if (isGrass == 1) {
        float maxGrassHeight = 15.0; 
        vec3 offset = vNormal * shellHeight * maxGrassHeight;
        float windFactor = pow(shellHeight, 2.0); 
        float windStrength = 2.5;
        float windX = sin(time * 2.0 + p.x * 0.05 + p.z * 0.05) * windStrength * windFactor;
        float windZ = cos(time * 1.5 + p.x * 0.05 - p.z * 0.05) * windStrength * windFactor;
        offset.x += windX;
        offset.z += windZ;
        p += offset;
    }

    vec4 worldPos = modelMatrix * vec4(p, 1.0);
    gl_Position = modelViewProjectionMatrix * vec4(p, 1.0);
    normal = normalize(vec3(normalMatrix * vec4(vertexNormal, 0.0)));
    pos = worldPos.xyz;
    localPos = p;
    localPosUndisplaced = origP; 
    texCoord = vTexCoord;
}