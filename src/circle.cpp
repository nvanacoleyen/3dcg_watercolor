#include "circle.h"

//Circle::Circle(Window* pWindow, glm::vec2 pos, float radius, int thickness, int vCount)
//    : Circle(pWindow, pos, radius, thickness, vCount) 
//{
//}

Circle::Circle(Window* pWindow, glm::vec3 pos, float radius, int thickness, int vCount)
    : m_pWindow(pWindow) 
    , m_position(pos)
    , m_radius(radius) 
    , m_thickness(thickness) 
    , m_vCount(vCount)    
{ 
    
    float angleIncrement = 2.0f * glm::pi<float>() / vCount;
    for (int t = 0; t < thickness; t++)
    {
        //    // Calculate the effective radius for the current layer
        float layerRadius = m_radius - (t / (800 / 2.0f)); // -(t / 100);   

        // positions
        for (int i = 0; i <= vCount; i++)
        {
            float currentAngle = angleIncrement * i;
            float x = layerRadius * cos(currentAngle) + m_position.x;
            float y = (layerRadius * sin(currentAngle) + m_position.y) * m_pWindow->getAspectRatio();
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(0.0f);
        }
    }
};

void Circle::buildCircle(int thickness, int vCount, float aspectRatio) 
{ 
    vertices = {};
    float angleIncrement = 2.0f * glm::pi<float>() / vCount;
    for (int t = 0; t < thickness; t++)
    {
        //    // Calculate the effective radius for the current layer
        float layerRadius = m_radius - (t / (800 / 2.0f)); // -(t / 100);   

        // positions
        for (int i = 0; i <= vCount; i++)
        {
            float currentAngle = angleIncrement * i;
            float x = layerRadius * cos(currentAngle) + m_position.x;
            float y = (layerRadius * sin(currentAngle) + m_position.y) * aspectRatio;
            vertices.push_back(x);  
            vertices.push_back(y);
            vertices.push_back(0.0f);
        }
    }
}

void Circle::updateCenter(glm::vec3 center)
{
    m_position = center;
}