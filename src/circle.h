#pragma once
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
DISABLE_WARNINGS_POP()
#include <framework/window.h>

class Circle {
public:
    Circle(Window* pWindow, glm::vec2 pos, float radius, int thickness, int vCount);

    void buildCircle(int thickness, int vCount, float aspectRatio); 
    void updateCenter(glm::vec2 center);

    glm::vec2 m_position;
    float m_radius;
    float m_thickness;
    float m_vCount;
    std::vector<float> vertices;  

    Window* m_pWindow;
};