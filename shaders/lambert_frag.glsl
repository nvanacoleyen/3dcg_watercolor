#version 430

// Global variables for lighting calculations
//layout(location = 1) uniform vec3 viewPos;

layout(location = 1) uniform vec3 lightPos;
layout(location = 2) uniform vec3 lightColor;
layout(location = 3) uniform vec3 kd;

// Output for on-screen color
layout(location = 0) out vec4 outColor;

// Interpolated output data from vertex shader
in vec3 fragPos; // World-space position
in vec3 fragNormal; // World-space normal

float normDot(vec3 N, vec3 L)
{
    vec3 nrmN = normalize(N); 
    vec3 nrmL = normalize(L);
    float result = dot(nrmN, nrmL);
    return max(result, 0.0);
}

void main()
{
    vec3 brightness = kd * normDot(lightPos - fragPos, fragNormal);  

    vec3 color = lightColor.x * brightness;

    outColor = vec4(color, 1.0); 
}