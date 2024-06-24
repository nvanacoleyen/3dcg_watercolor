#include "cell.h"
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
// Library for UI
#include <imgui/imgui.h>
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

bool calculate_watercolour = false;
float brush_radius = 20;

int main()
{
    Window window{ "Watercolor", glm::ivec2(WIDTH, HEIGHT), OpenGLVersion::GL45 };
    Camera camera{ &window, glm::vec3(393.572052f, 290.958832f, 737.367920f), glm::vec3(0.00720430166f, 0.0117728803f, -0.999904811f) };

    constexpr float fov     = glm::pi<float>() / 4.0f;
    constexpr float aspect  = static_cast<float>(WIDTH) / static_cast<float>(HEIGHT);
    const glm::mat4 mainProjectionMatrix = glm::perspective(fov, aspect, 0.1f, 1000.0f);
    float aspectRatio       = window.getAspectRatio();

    window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT); 
    bool mouseMenu = false;

    /* Create lightsource */
    Light light = Light{ glm::vec3(800, 800, -1000), glm::vec3(1.0) };

    /* GENERATE HEIGHTMAP */
    std::vector<double> paperPresets[] = { 
        { 7.0, 2.1, 3.0, 2.0 }, // Preset 1 (octaves, lucanarity, gain, maxHeight)
        { 6.0, 3.0, 2.0, 5.0 }, // Preset 2
        { 5.0, 3.5, 1.0, 5.0 }  // Preset 3 
    };
    int currentPreset = 0;

    float minHeight         = 0.f;
    float maxHeightSlider   = 6 * paperPresets[0][3];
    float heightRatio       = 1;
    Terrain paper1(paperPresets[0][0], paperPresets[0][1], paperPresets[0][2], HEIGHT, WIDTH, minHeight, paperPresets[0][3]);
    Terrain paper2(paperPresets[1][0], paperPresets[1][1], paperPresets[1][2], HEIGHT, WIDTH, minHeight, paperPresets[1][3]);
    Terrain paper3(paperPresets[2][0], paperPresets[2][1], paperPresets[2][2], HEIGHT, WIDTH, minHeight, paperPresets[2][3]); 
    std::vector<Terrain> papers{ paper1, paper2, paper3 }; 
    //Terrain& currentPaper = papers[currentPreset];  
    

    /* GENERATE GRID OF CELLS */
    Staggered_Grid x_velocity(WIDTH, HEIGHT, true);
    Staggered_Grid y_velocity(WIDTH, HEIGHT, false);
    std::vector<float> water_pressure(WIDTH * HEIGHT, 0.f);
    //std::vector<Cell> Grid;
    for (auto& paper : papers) {
        for (int j = 0; j < HEIGHT; j++) {
            for (int i = 0; i < WIDTH; i++) {
                paper.Grid.push_back(Cell(glm::vec3(i, j, paper.getHeightmap()[j][i]), 1));  
            }
        }
    }
    
    //updateColors(papers[0].getMesh().vertices, Grid, brush_radius, papers[0].getVBO()); 

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
    for (auto& paper : papers) {
        paper.createGLState();
    }

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
                                papers[currentPreset].Grid[WIDTH * j + i].m_waterConc = 1;
                            }
                            /* When using colorbrush */
                            else {
                                papers[currentPreset].Grid[WIDTH * j + i].m_pigmentConc = 1;
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

        ImGui::Begin("Window");
        ImGui::DragFloat("Max height paper", &papers[currentPreset].maxHeight, 0.1f, 0.01f, maxHeightSlider, "MaxHeight: %.2f %%");
        ImGui::DragInt("Paper presets", &currentPreset, 0.01, 0, 2); 
        ImGui::DragFloat("LightPos.x", &light.position.x, 10, 0, 1500);   
        ImGui::DragFloat("LightPos.y", &light.position.y, 10, 0, 1500);
        ImGui::DragFloat("LightPos.z", &light.position.z, 10, -1500, 0); 
        ImGui::End(); 

        heightRatio = papers[currentPreset].maxHeight / papers[currentPreset].oldMaxHeight; 

        /* If you drag menu slider, when released it will update vertices paper */
        if (!ImGui::GetIO().WantCaptureMouse && mouseMenu == true) { 
            /* Update vertices and buffer of current paper*/
            papers[currentPreset].updateVertices(papers[currentPreset].maxHeight, heightRatio);
            papers[currentPreset].updateBuffer();
            /* Update Grid cell positions */
            for (int j = 0; j < HEIGHT; j++) {
                for (int i = 0; i < WIDTH; i++) {
                    papers[currentPreset].Grid[j * WIDTH + i].m_position.z = papers[currentPreset].getHeightmap()[j][i];
                }
            }
        }
        /* If mouse uses menu, mouseMenu == true, else == false */
        if (ImGui::GetIO().WantCaptureMouse) { mouseMenu = true; }
        else { mouseMenu = false; }
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Enable color writes. 

        /* You toggle these by pressing enter. */
        if (calculate_watercolour) { 
            /* Move water functions. */
            UpdateVelocities(&papers[currentPreset].Grid, &x_velocity, &y_velocity, &water_pressure);
            RelaxDivergence(&x_velocity, &y_velocity, &water_pressure);
            FlowOutward(&papers[currentPreset].Grid, &water_pressure);
        
            /* Pigment functions */
            movePigment(&papers[currentPreset].Grid, &x_velocity, &y_velocity);
            updateColors(papers[currentPreset].getMesh().vertices, papers[currentPreset].Grid, brush_radius, papers[currentPreset].getVBO());
        }
        /* Brush function */
        else if (isDragging) {
            glm::vec2 cursorPosition = window.getCursorPos() / window.getDpiScalingFactor();
            updateColors(papers[currentPreset].getMesh().vertices, papers[currentPreset].Grid, brush_radius, papers[currentPreset].getVBO());
        }

        // Update the buffer data 
        papers[currentPreset].updateBuffer();
         
        const glm::mat4 mvp = mainProjectionMatrix * camera.viewMatrix();

        /* RENDER PAPER */
        paperShader.bind();
        {
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvp));
            glUniform3fv(1, 1, glm::value_ptr(light.position));
            glUniform3fv(2, 1, glm::value_ptr(light.color)); 
            papers[currentPreset].Render();
        }

        glfwPollEvents();

        // Present result to the screen.
        window.swapBuffers();
    }
    // Clean up
    for (auto& paper : papers) {
        glDeleteVertexArrays(1, &paper.getVAO());
        glDeleteBuffers(1, &paper.getVBO());
    }

    return 0;
}