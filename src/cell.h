#pragma once
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
DISABLE_WARNINGS_POP()
#include <framework/window.h>

class Cell {
public:

    Cell(glm::vec3 pos, float size)
        : m_position(pos), m_size(size), is_wet(false) {};

    glm::vec3 m_position;
    float m_size;
    float m_pigmentConc{ 0 };
    float m_waterConc{ 0 };
    double m_height{ 0 };
    bool is_wet;
}; 