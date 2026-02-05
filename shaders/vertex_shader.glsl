#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float aShadow;      // 0.0 or 1.0 (or intensity)
layout (location = 2) in float aTemperature; // temperature value

out float vShadow;
out float vTemperature;

uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * vec4(aPos, 1.0);
    vShadow = aShadow;
    vTemperature = aTemperature;
}