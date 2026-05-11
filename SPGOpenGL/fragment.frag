#version 400
out vec4 fragColor;

in vec3 normal;
in vec3 pos;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 objectColor; // Aici primim corect culoarea din C++

vec3 lighting(vec3 objectColor, vec3 pos, vec3 normal, vec3 lightPos, vec3 viewPos,
              vec3 ambient, vec3 lightColor, vec3 specular, float specPower)
{
    vec3 L = normalize(lightPos - pos);
    vec3 V = normalize(viewPos - pos);
    vec3 N = normalize(normal);
    vec3 R = reflect(-L, N);

    float diffCoef = max(dot(N, L), 0.0);
    float specCoef = pow(max(dot(V, R), 0.0), specPower);

    vec3 ambientColor = ambient * lightColor;
    vec3 diffuseColor = diffCoef * lightColor;
    vec3 specularColor = specCoef * specular * lightColor;

    vec3 col = (ambientColor + diffuseColor + specularColor) * objectColor; 
    return clamp(col, 0.0, 1.0);
}

void main() 
{
    // Am sters for?area objectColor la alb de aici!
    
    vec3 ambient = vec3(0.3);  // Umbre pu?in mai luminoase
    vec3 specular = vec3(0.1); // Am sc?zut luciul (iarba ?i lemnul nu prea lucesc)
    float specPower = 16.0;
    
    // Lumina principal?: Alb-G?lbui (Soare)
    vec3 lightColor1 = vec3(1.0, 0.95, 0.9); 
    vec3 color1 = lighting(objectColor, pos, normal, lightPos, viewPos, ambient, lightColor1, specular, specPower);
    
    // Lumina secundar? (lumin? de umplere)
    vec3 lightColor2 = vec3(0.2, 0.2, 0.2); 
    vec3 color2 = lighting(objectColor, pos, normal, viewPos, viewPos, vec3(0.0), lightColor2, specular, specPower);
    
    vec3 finalColor = clamp(color1 + color2, 0.0, 1.0);
    fragColor = vec4(finalColor, 1.0);
}