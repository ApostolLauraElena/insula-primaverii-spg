#version 400
out vec4 fragColor;

in vec3 normal;
in vec3 pos;
in vec2 texCoord;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 objectColor;

// Uniforms pentru iarba
uniform int isGrass;
uniform float shellHeight;
uniform sampler2D noiseTexture;

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
    if (isGrass == 1 && shellHeight > 0.0) {
        vec2 uv1 = texCoord * 17.0;
vec2 uv2 = texCoord * 43.0 + vec2(0.37, 0.71);

float n1 = texture(noiseTexture, uv1).r;
float n2 = texture(noiseTexture, uv2).r;
float noiseVal = mix(n1, n2, 0.35);

        if (noiseVal < pow(shellHeight, 1.2)) {
            discard;
        }
    }

    vec3 ambient = vec3(0.3);
     vec3 specular = (isGrass == 1) ? vec3(0.0) : vec3(0.1); 
    
    vec3 finalColor = objectColor;
    if (isGrass == 1) {
        vec3 rootColor = vec3(0.07, 0.18, 0.05);
        vec3 tipColor = vec3(0.34, 0.58, 0.22);
        float colorCurve = pow(shellHeight, 0.6);
        float colorVariation = texture(noiseTexture, texCoord * 7.0 + vec2(0.19, 0.43)).r;
        finalColor = mix(rootColor, tipColor, colorCurve) * mix(0.82, 1.08, colorVariation);
    }

    vec3 color1 = lighting(finalColor, pos, normal, lightPos, viewPos,
                           ambient, vec3(1.0, 0.95, 0.9), specular, 16.0);
    vec3 color2 = lighting(finalColor, pos, normal, viewPos, viewPos,
                           vec3(0.0), vec3(0.2), specular, 4.0);
    fragColor = vec4(clamp(color1 + color2, 0.0, 1.0), 1.0);

    vec3 litColor = clamp(color1 + color2, 0.0, 1.0);

    float dist = distance(viewPos, pos);
    float fog = smoothstep(450.0, 1200.0, dist);

    vec3 skyColor = vec3(0.5, 0.8, 0.9);
    vec3 finalFogColor = mix(litColor, skyColor, fog * 0.45);

    fragColor = vec4(finalFogColor, 1.0);

}