#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 cameraPos;

void main() {
    vec3 waterColor = vec3(0.2, 0.6, 0.8);
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.5)); // Luz
    vec3 ambient = vec3(0.1, 0.2, 0.3); // Luz ambiental

    // Iluminación Difusa
    float diffuse = max(dot(Normal, lightDir), 0.0);
    
    // Iluminación Especular (brillos)
    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, Normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); 
    vec3 specular = vec3(1.0) * spec; // Brillos blancos

    vec3 finalColor = ambient + (waterColor * diffuse) + specular;

    // AÑADIMOS TRANSPARENCIA
    FragColor = vec4(finalColor, 0.85); // 0.85 de opacidad
}