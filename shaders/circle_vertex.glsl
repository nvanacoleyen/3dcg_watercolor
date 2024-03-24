#version 430

// Per-vertex attributes
layout(location = 0) in vec3 aPos; // World-space position

layout(location = 3) uniform vec2 uCursorPosition;

void main() {
	// Transform 3D position into on-screen position
    vec3 transformedPosition = vec3(aPos.xy + uCursorPosition, aPos.z);
    gl_Position = vec4(transformedPosition, 1.0);
}