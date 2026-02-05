#version 330 core

in float vShadow;
in float vTemperature;

out vec4 FragColor;

uniform int visualizationMode; // 0=wireframe, 1=shadow, 2=temperature
uniform vec4 wireframeColor;   // grey color for wireframe

// Temperature gradient (you can make these uniforms too)
vec3 coldColor = vec3(0.0, 0.0, 1.0);  // blue
vec3 hotColor = vec3(1.0, 0.0, 0.0);   // red

vec3 fullShadow = vec3(1.0, 0.0, 0.0);
vec3 fullLight = vec3(1.0, 1.0, 1.0);
vec3 color;

void main() {
    if (visualizationMode == 0) {
        // Wireframe mode - solid grey
        FragColor = wireframeColor;
    }
    else if (visualizationMode == 1) {
        // Shadow mode - red (shadow) or white (lit)
        //FragColor = vShadow > 0.0 ? vec4(1.0, 0.0, 0.0, 1.0) : vec4(1.0, 1.0, 1.0, 1.0);
        color = mix(fullLight, fullShadow, vShadow);
        FragColor = vec4(color, 1.0);

    }
    else if (visualizationMode == 2) {
        // Temperature mode - gradient
        color = mix(coldColor, hotColor, vTemperature);
        FragColor = vec4(color, 1.0);
    }
}