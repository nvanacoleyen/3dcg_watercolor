#version 450 core

layout(location = 0) uniform mat4 mvp;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

out vec3 fragPos;
out vec3 fragNormal;
out vec3 ourColor;

void main()
{
    gl_Position = mvp * vec4(aPos.x, aPos.y, aPos.z, 1.0);

    fragPos = aPos;
    fragNormal = aNormal;
    ourColor = aColor;
}