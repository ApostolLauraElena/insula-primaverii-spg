#version 400

in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;

out vec4 fragColor;

uniform sampler2D ciresTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    vec4 texColor = texture(ciresTexture, texCoord);
    
    if (texColor.a < 0.1) {
        discard;
    }
    
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    
    vec3 ambient = vec3(0.25) * texColor.rgb;
    vec3 diffuse = diff * texColor.rgb * vec3(1.0, 0.95, 0.9);
    
    fragColor = vec4(ambient + diffuse, texColor.a);
}