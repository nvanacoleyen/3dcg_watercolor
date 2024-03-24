#version 450 core

layout(location = 2) uniform vec3 color;

out vec4 FragColor;

void main() {
    FragColor = vec4(color, 1.0); // Red color for the line
}