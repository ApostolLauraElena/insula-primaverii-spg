#version 400
out vec4 fragColor;

in vec3 normal;
in vec3 pos;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 objectColor;

vec3 lighting(vec3 objColor, vec3 p, vec3 n, vec3 lPos, vec3 vPos,
              vec3 ambient, vec3 lightColor, vec3 specular, float specPower)
{
    vec3 L = normalize(lPos - p);
    vec3 V = normalize(vPos - p);
    vec3 N = normalize(n);
    vec3 R = reflect(-L, N);
    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(V, R), 0.0), specPower);
    vec3 col = (ambient * lightColor + diff * lightColor + spec * specular * lightColor) * objColor;
    return clamp(col, 0.0, 1.0);
}

void main()
{
    vec3 ambient = vec3(0.3);
    vec3 specular = vec3(0.1);
    vec3 color1 = lighting(objectColor, pos, normal, lightPos, viewPos,
                           ambient, vec3(1.0, 0.95, 0.9), specular, 16.0);
    vec3 color2 = lighting(objectColor, pos, normal, viewPos, viewPos,
                           vec3(0.0), vec3(0.2), specular, 4.0);
    fragColor = vec4(clamp(color1 + color2, 0.0, 1.0), 1.0);
}
