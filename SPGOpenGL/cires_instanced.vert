#version 400

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;
layout(location = 3) in mat4 instanceMatrix; 

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix; 

out vec2 texCoord;
out vec3 normal;
out vec3 fragPos;

void main() {
    texCoord = vTexCoord;

    mat4 normalMatrix = transpose(inverse(modelMatrix * instanceMatrix));
    normal = normalize((normalMatrix * vec4(vNormal, 0.0)).xyz);
    
    vec4 worldPos = modelMatrix * instanceMatrix * vec4(vPos, 1.0);
    fragPos = worldPos.xyz;
    
    gl_Position = projectionMatrix * viewMatrix * worldPos;
}