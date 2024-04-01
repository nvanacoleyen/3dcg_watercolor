#version 450 core

out vec4 FragColor;

in vec3 fragPos;
in vec3 fragNormal;
in vec3 ourColor;

float normDot(vec3 N, vec3 L)
{
    vec3 nrmN = normalize(N);
    vec3 nrmL = normalize(L);
    float result = dot(nrmN, nrmL);
    return max(result, 0.0);
}

void main()
{
    vec3 lightPos = vec3(800, 800, 100);
    vec3 lightDir = normalize(lightPos - fragPos);
    float brightness = 1.0 * normDot(lightDir, fragNormal);
    brightness = clamp(brightness, 0.0, 1.0);

    vec3 finalColor = ourColor * brightness;

    FragColor = vec4(finalColor, 1.0f);
}