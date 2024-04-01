#version 450 core

layout(location = 0) uniform mat4 mvp;

layout(location = 0) in vec3 aPos;

// Output variables
out vec3 fragPos;  // Fragment position in world coordinates

void main()
{
    gl_Position = mvp * vec4(aPos, 1.0);
    fragPos = aPos;
}