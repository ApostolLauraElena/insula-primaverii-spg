#version 400

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord; // Coordonatele UV

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelMatrix;
uniform mat4 normalMatrix;
uniform float time;
uniform int isWater;

// Uniforms pentru iarba
uniform int isGrass;
uniform float shellHeight;

out vec3 normal;
out vec3 pos;
out vec2 texCoord;

void main()
{
    vec3 p = vPos;

    if (isWater == 1) {
        p.y += 1.5 * sin(0.05 * p.x + time)
             + 1.5 * cos(0.05 * p.z + time * 0.7);
    }

    if (isGrass == 1) {
        float maxGrassHeight = 15.0; 
        
        vec3 offset = vNormal * shellHeight * maxGrassHeight;

       float windFactor = pow(shellHeight, 2.0); 
        float windStrength = 2.5; // C�t de tare bate v�ntul
        
        float windX = sin(time * 2.0 + p.x * 0.05 + p.z * 0.05) * windStrength * windFactor;
        float windZ = cos(time * 1.5 + p.x * 0.05 - p.z * 0.05) * windStrength * windFactor;
        
        offset.x += windX;
        offset.z += windZ;
       
        p += offset;
    }

    vec4 worldPos = modelMatrix * vec4(p, 1.0);
    gl_Position = modelViewProjectionMatrix * vec4(p, 1.0);
    normal = vec3(normalMatrix * vec4(vNormal, 0.0));
    pos = worldPos.xyz;
    texCoord = vTexCoord;
}