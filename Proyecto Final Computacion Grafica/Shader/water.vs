#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

void main() {
    // 1. Mover el vértice para la ola
    vec3 animatedPos = aPos;
    float waveX = 0.3 * sin(aPos.x * 2.0 + time);
    float waveZ = 0.2 * cos(aPos.z * 2.0 + time);
    animatedPos.y += waveX + waveZ;

    // 2. Recalcular la normal basada en las olas
    float derivX = 0.3 * 2.0 * cos(aPos.x * 2.0 + time);
    float derivZ = -0.2 * 2.0 * sin(aPos.z * 2.0 + time);

    vec3 tangentX = normalize(vec3(1.0, derivX, 0.0));
    vec3 tangentZ = normalize(vec3(0.0, derivZ, 1.0));
    
    vec3 newNormal = normalize(cross(tangentZ, tangentX));

    FragPos = vec3(model * vec4(animatedPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * newNormal;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}