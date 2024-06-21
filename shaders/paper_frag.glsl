#version 450 core

out vec4 FragColor;

in vec3 fragPos;
in vec3 fragNormal;
in vec3 ourColor;

layout(location = 1) uniform vec3 lightPos;
layout(location = 2) uniform vec3 lightColor;

float normDot(vec3 N, vec3 L)
{
    vec3 nrmN = normalize(N);
    vec3 nrmL = normalize(L);
    float result = dot(nrmN, nrmL);
    return max(result, 0.0);
}

void main()
{
    vec3 lightDir = normalize(lightPos - fragPos);
    float brightness = 1.0 * normDot(lightDir, fragNormal);

    vec3 finalColor = ourColor * brightness;

    FragColor = vec4(finalColor, 1.0f);
}