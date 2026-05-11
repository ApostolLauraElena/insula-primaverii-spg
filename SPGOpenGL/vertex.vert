#version 400

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 normalMatrix;
uniform float time;
uniform int isWater;

out vec3 normal;
out vec3 pos;

void main()
{
    vec3 p = vPos;

    if (isWater == 1) {
        p.y += 1.5 * sin(0.05 * p.x + time)
             + 1.5 * cos(0.05 * p.z + time * 0.7);
    }

    gl_Position = modelViewProjectionMatrix * vec4(p, 1.0);
    normal = vec3(normalMatrix * vec4(vNormal, 1.0));
    pos = p;
}
