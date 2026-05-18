#version 400
out vec4 fragColor;

in vec3 normal;
in vec3 pos;
in vec3 localPos;
in vec2 texCoord;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 objectColor;

uniform int isGrass;
uniform int isWater;
uniform float shellHeight;
uniform float time;
uniform float waterLevel;
uniform float terrainSize;
uniform sampler2D noiseTexture;       
uniform sampler2D waterNormalTexture; 

uniform int isSun;
uniform sampler2D sunTexture;

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
    // 0. Logica pentru Soare Billboard
    if (isSun == 1) {
        vec4 texColor = texture(sunTexture, texCoord);
    
        if (texColor.a < 0.05) discard; 
    
        fragColor = texColor;
        return;
    }
    // 1. Logica pentru Apa 
    if (isWater == 1) {
        vec2 uvA = texCoord * 22.0 + vec2(time * 0.035, -time * 0.018);
        vec2 uvB = texCoord * 41.0 + vec2(time * 0.020, time * 0.045);

        vec3 nA = texture(waterNormalTexture, uvA).rgb * 2.0 - 1.0;
        vec3 nB = texture(waterNormalTexture, uvB).rgb * 2.0 - 1.0;
        vec3 detailNormal = normalize(vec3(nA.x + nB.x * 0.55, nA.y + nB.y * 0.55, nA.z + nB.z));
        vec3 detailWorld = normalize(vec3(detailNormal.x * 0.42, detailNormal.z, detailNormal.y * 0.42));
        vec3 N = normalize(normal * 0.75 + detailWorld * 0.55);

        vec3 L = normalize(lightPos - pos);
        vec3 V = normalize(viewPos - pos);
        vec3 H = normalize(L + V);
        float diff = max(dot(N, L), 0.0);
        float spec = pow(max(dot(N, H), 0.0), 96.0);
        float fresnel = pow(1.0 - clamp(dot(N, V), 0.0, 1.0), 5.0);
        float crest = smoothstep(waterLevel - 0.4, waterLevel + 1.4, pos.y);

        vec3 deepColor = vec3(0.05, 0.20, 0.45);   
        vec3 shallowColor = vec3(0.30, 0.55, 0.85);
        vec3 waterColor = mix(deepColor, shallowColor, crest);
        waterColor *= 0.55 + diff * 0.55;
        waterColor += vec3(1.0, 0.96, 0.82) * spec * 1.15;
        waterColor += vec3(0.45, 0.75, 0.95) * fresnel * 0.55;

        fragColor = vec4(clamp(waterColor, 0.0, 1.0), mix(0.58, 0.82, fresnel));
        return;
    }

    // 2. Logica pentru Iarba 
    if (isGrass == 1 && shellHeight > 0.0) {
        vec2 uv1 = texCoord * 17.0;
        vec2 uv2 = texCoord * 43.0 + vec2(0.37, 0.71);

        float n1 = texture(noiseTexture, uv1).r;
        float n2 = texture(noiseTexture, uv2).r;
        float noiseVal = mix(n1, n2, 0.35);

        // Daca valoarea de noise e mai mica decat inaltimea stratului curent, „gaurim” stratul
        if (noiseVal < pow(shellHeight, 1.2)) {
            discard; 
        }
    }

    // Iluminare si colorare teren/iarba
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

    vec3 color1 = lighting(finalColor, pos, normal, lightPos, viewPos, ambient, vec3(1.0, 0.95, 0.9), specular, 16.0);
    vec3 color2 = lighting(finalColor, pos, normal, viewPos, viewPos, vec3(0.0), vec3(0.2), specular, 4.0);
    vec3 litColor = clamp(color1 + color2, 0.0, 1.0);

    float dist = distance(viewPos, pos);
    float fog = smoothstep(450.0, 1200.0, dist);
    vec3 skyColor = vec3(0.5, 0.8, 0.9);
    vec3 finalFogColor = mix(litColor, skyColor, fog * 0.45);

    fragColor = vec4(finalFogColor, 1.0);
}