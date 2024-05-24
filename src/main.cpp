#include "cell.h"
#include "heightmap.h"
#include "staggered_grid.h"
#include "global_constants.h"
#include "move_water.h"
#include "move_pigment.h"
#include "camera.h"
#include "paper/terrain.h" 
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
// Include GLEW before GLFW
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
// Library for loading an image
#include <stb/stb_image.h>
DISABLE_WARNINGS_POP()
#include <array>
#include <framework/mesh.h>
#include <framework/shader.h>
#include <framework/trackball.h>
#include <framework/window.h>
#include <framework/vertexbuffer.h>
#include <framework/vertexarray.h>
#include <framework/texture.h>

#include <iostream> 
#include <span>
#include <vector>
#include <memory> 

bool cursorCircle = true;
bool waterBrush = true;
bool colorBrush = false;
bool isDragging = false;


struct Light {
    glm::vec3 position;
    glm::vec3 color;
};
std::vector lights{ Light { glm::vec3(0, 0, 3), glm::vec3(1) } };

bool calculate_watercolour = false;
float brush_radius = 20;

void updateColors(std::vector<paperVertex>& vertices, std::vector<Cell>& Grid, float& brush_radius, GLuint& VBO)
{
    // Color with/without water/pigment concentration
    for (size_t i = 0; i < vertices.size(); i ++)
    {   // For every vertex in the square:
        glm::vec3 color; 
        if (Grid[i].m_waterConc == 1 && Grid[i].m_pigmentConc == 0) {
            color = glm::vec3(0.5);
        }
        else if (Grid[i].m_pigmentConc != 0) {
            float pigment_factor = std::min(1.f, std::max(0.f, Grid[i].m_pigmentConc));
            color = glm::vec3(0.5 - (0.5 * pigment_factor), 0.5 - (0.5 * pigment_factor), 0.5 + (0.5 * pigment_factor));
        }
        else {
            color = vertices[i].color;
        }
        vertices[i].color = color;   
    }
}


int main()
{
    Window window{ "Watercolor", glm::ivec2(WIDTH, HEIGHT), OpenGLVersion::GL45 };
    Camera camera{ &window, glm::vec3(393.572052f, 290.958832f, 737.367920f), glm::vec3(0.00720430166f, 0.0117728803f, -0.999904811f) };
    constexpr float fov = glm::pi<float>() / 4.0f;
    constexpr float aspect = static_cast<float>(WIDTH) / static_cast<float>(HEIGHT);
    const glm::mat4 mainProjectionMatrix = glm::perspective(fov, aspect, 0.1f, 1000.0f);
    float aspectRatio = window.getAspectRatio();
    window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);

    glm::vec3 lightPos(800, 800, -1000);

    /* GENERATE HEIGHTMAP */
    int octaves = 7;
    double lucanarity = 2.1042;
    double gain = 3;

    Terrain paper(octaves, lucanarity, gain, HEIGHT, WIDTH, 0, 10);

    // Get references to variables paper
    paperMesh &paper_mesh = paper.getMesh();  
    std::vector<std::vector<double>> &heightmap = paper.getHeightmap(); 
    GLuint &paper_vao = paper.getVAO();
    GLuint &paper_vbo = paper.getVBO();

    /* GENERATE GRID OF CELLS */
    Staggered_Grid x_velocity(WIDTH, HEIGHT, true);
    Staggered_Grid y_velocity(WIDTH, HEIGHT, false);
    std::vector<float> water_pressure(WIDTH * HEIGHT, 0.f);

    std::vector<Cell> Grid;
    for (int j = 0; j < HEIGHT; j++) {
        for (int i = 0; i < WIDTH; i++) {
            Grid.push_back(Cell(glm::vec3(i, j, heightmap[j][i]), 1));
        }
    }
    
    updateColors(paper_mesh.vertices, Grid, brush_radius, paper_vbo);  

    // Key handle function
    window.registerKeyCallback([&](int key, int /* scancode */, int action, int /* mods */) {
        if (action != GLFW_RELEASE)
            return;

        const bool shiftPressed = window.isKeyPressed(GLFW_KEY_LEFT_SHIFT) || window.isKeyPressed(GLFW_KEY_RIGHT_SHIFT);

        switch (key) {
        case GLFW_KEY_B:
            waterBrush = !waterBrush;
            break;
        case GLFW_KEY_ENTER:
            calculate_watercolour = !calculate_watercolour;
            break;
        case GLFW_KEY_LEFT:
            if (brush_radius > 0) {
                brush_radius -= 1;
            }
            break;
        case GLFW_KEY_RIGHT:
            if (brush_radius < 50) {
                brush_radius += 1;
            }
            break;
        default:
            break;
        };
    });

    window.registerMouseButtonCallback([&](int button, int action, int mods) {

        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (action == GLFW_PRESS) {
                isDragging = true;
            }
            else if (action == GLFW_RELEASE) {
                isDragging = false;
            }
        }
    });

    /* SHADERS */
    const Shader paperShader = ShaderBuilder().addStage(GL_VERTEX_SHADER, "shaders/paper_vertex.glsl").addStage(GL_FRAGMENT_SHADER, "shaders/paper_frag.glsl").build();

    // Create buffers for terrain
    paper.createGLState();

    // Draw with cursor
    window.registerMouseMoveCallback([&](const glm::vec2& cursorPos) {
        glm::vec2 cursorPosition = window.getCursorPos() / window.getDpiScalingFactor();
        /* If right mouse button is pressed */
        if (isDragging) {
            /* If within brush radius */
            for (int j = cursorPosition.y - brush_radius; j <= cursorPosition.y + brush_radius; j++) {
                for (int i = cursorPosition.x - brush_radius; i <= cursorPosition.x + brush_radius; i++) {
                    /* IF not outside the range of */
                    if (j < HEIGHT && i < WIDTH && j >= 0 && i >= 0) {
                        float dist = sqrt(pow(i - cursorPosition.x, 2) + pow(j - cursorPosition.y, 2));
                        
                        if (dist <= brush_radius) {
                            /* When using waterbrush */
                            if (waterBrush) {
                                Grid[WIDTH * j + i].m_waterConc = 1;
                            }
                            /* When using colorbrush */
                            else {
                                Grid[WIDTH * j + i].m_pigmentConc = 1;
                            }
                        }
                    }
                }
            }
        }
    });

    // Main loop
    while (!window.shouldClose()) {
        window.updateInput();
        camera.updateInput();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Enable color writes. 

        /* You toggle these by pressing enter. */
        if (calculate_watercolour) {
            /* Move water functions. */
            UpdateVelocities(&Grid, &x_velocity, &y_velocity, &water_pressure);
            RelaxDivergence(&x_velocity, &y_velocity, &water_pressure);
            FlowOutward(&Grid, &water_pressure);
        
            /* Pigment functions */
            movePigment(&Grid, &x_velocity, &y_velocity);
            updateColors(paper_mesh.vertices, Grid, brush_radius, paper_vbo);  
        }
        /* Brush function */
        else if (isDragging) {
            glm::vec2 cursorPosition = window.getCursorPos() / window.getDpiScalingFactor();
            updateColors(paper_mesh.vertices, Grid, brush_radius, paper_vbo);  
            
        }

        // Update the buffer data 
        paper.updateBuffer(); 
         
        const glm::mat4 mvp = mainProjectionMatrix * camera.viewMatrix();

        /* RENDER PAPER */
        paperShader.bind();
        {
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvp));
            glUniform3fv(1, 1, glm::value_ptr(lightPos));    
            paper.Render();
        }

        glfwPollEvents();

        // Present result to the screen.
        window.swapBuffers();
    }
    // Clean up
    glDeleteVertexArrays(1, &paper_vao); 
    glDeleteBuffers(1, &paper_vbo);  

    return 0;
}