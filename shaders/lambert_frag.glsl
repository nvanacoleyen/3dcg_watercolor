#version 430

// Input variables
in vec3 fragPos;  // Fragment position in world coordinates

// Output variable
out vec4 fragColor;  // Final fragment color

void main()
{
    // Calculate fragment color based on height or other factors
    fragColor = vec4(1.0, 1.0, 1.0, 1.0);  // White color for now 
}