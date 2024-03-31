#include "cell.h"
#include "circle.h"
#include "heightmap.h"
#include "staggered_grid.h"
#include "global_constants.h"
#include "move_water.h"
#include "move_pigment.h"
#include "camera.h"
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
bool calculate_watercolour = false;


int main()
{
    Window window{ "Watercolor", glm::ivec2(WIDTH, HEIGHT), OpenGLVersion::GL45 };
    Camera camera{ &window, glm::vec3(393.572052f, 290.958832f, 737.367920f), glm::vec3(0.00720430166f, 0.0117728803f, -0.999904811f) };
    constexpr float fov = glm::pi<float>() / 4.0f;
    constexpr float aspect = static_cast<float>(WIDTH) / static_cast<float>(HEIGHT);
    const glm::mat4 mainProjectionMatrix = glm::perspective(fov, aspect, 0.1f, 1000.0f);
    float aspectRatio = window.getAspectRatio();
    window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);

    std::cout << "aspectratio: " << aspectRatio << "\n";

    Circle cursorCircle(&window, glm::vec2(0.0f), 5, 4, 200); 

    int octaves = 5;
    double lucanarity = 2.1042;
    double gain = 0.6;
    std::vector<std::vector<double>> heightmap = generatePerlinNoise(WIDTH, HEIGHT, octaves, lucanarity, gain);  
    // Normalize heightmap values to range [0, 1]
    normalizeHeightmap(heightmap);  
    //visualizeHeightmap(heightmap);   
    //std::vector<float> createHeightmapVertices(const char* imagePath);
    //std::vector<unsigned int> createHeightmapIndices(const char* imagePath);
    //void drawHeightmap(const char* imagePath, std::vector<float> vertices, std::vector<unsigned int> indices);

    // Create grid of cells
    Staggered_Grid x_velocity(WIDTH, HEIGHT, true);
    Staggered_Grid y_velocity(WIDTH, HEIGHT, false);
    std::vector<float> water_pressure(WIDTH * HEIGHT, 0.f);

    // Create grid of cells
    std::vector<Cell> Grid;
    for (int j = 0; j < HEIGHT; j++) {
        for (int i = 0; i < WIDTH; i++) {
            Grid.push_back(Cell(glm::vec2(i, j), 1, heightmap[j][i]));
        }
    }

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
            calculate_watercolour= !calculate_watercolour;
            break;
        default:
            break;
        };
    });

    const Shader circleShader = ShaderBuilder().addStage(GL_VERTEX_SHADER, "shaders/circle_vertex.glsl").addStage(GL_FRAGMENT_SHADER, "shaders/circle_frag.glsl").build(); 
    const Shader paperShader = ShaderBuilder().addStage(GL_VERTEX_SHADER, "shaders/paper_vertex.glsl").addStage(GL_FRAGMENT_SHADER, "shaders/paper_frag.glsl").build();
    
    // Enable depth testing.
    glEnable(GL_DEPTH_TEST);  

    VertexBuffer vboCircle(cursorCircle.vertices.data(), sizeof(float) * cursorCircle.vertices.size());
    VertexArray vaoCircle;
    vaoCircle.Bind();
    vaoCircle.AddBuffer(vboCircle, 0, 3, GL_FLOAT, 3 * sizeof(float));
    vaoCircle.Unbind(); 

    // Main loop
    while (!window.shouldClose()) {
        window.updateInput();
        camera.updateInput();
        glm::vec2 cursorPos = window.getCursorPos();
        //cursorPos = (cursorPos - glm::vec2(window.getWindowSize()) * 0.5f) / (glm::vec2(window.getWindowSize()) * 0.5f);

        cursorCircle.updateCenter(cursorPos);

        //glViewport(0, 0, window.getWindowSize().x, window.getWindowSize().y);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // If LMB clicked -> m_pigmentConc of every cell within radius is 1.0
        if (window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
            for (int j = cursorPos.y - cursorCircle.m_radius; j <= cursorPos.y + cursorCircle.m_radius; j++) { 
                for (int i = cursorPos.x - cursorCircle.m_radius; i <= cursorPos.x + cursorCircle.m_radius; i++) {
                    if (waterBrush) {
                        Grid[WIDTH * j + i].m_waterConc = 1;
                        Grid[WIDTH * j + i].is_wet = true;
                    }
                    else {
                        Grid[WIDTH * j + i].m_pigmentConc = 0.5;
                    }
                }
            }
        }
        
 
        /* You toggle these by pressing enter. */
        if (calculate_watercolour) {
            /* Move water functions. */
            UpdateVelocities(&Grid, &x_velocity, &y_velocity, &water_pressure);
            RelaxDivergence(&x_velocity, &y_velocity, &water_pressure);
            /* Here we would do flow outward if we are implementing that function */

            /* Pigment functions */
            movePigment(&Grid, &x_velocity, &y_velocity);
        }

        


        const glm::mat4 mvp = mainProjectionMatrix * camera.viewMatrix();
        
        /* RENDER GRID */
        std::vector<float> vertices;
        for (const auto& cell : Grid) {

            glm::vec3 color;

            if (cell.m_waterConc == 1 && cell.m_pigmentConc == 0) {
                color = glm::vec3(0.7);
            }
            else if (cell.m_pigmentConc != 0) {
                color = glm::vec3(cell.m_pigmentConc, 0, 0);
            }
            else {
                color = glm::vec3(1);
            }

            vertices.insert(vertices.end(), {
                cell.m_position.x, cell.m_position.y, 0.0f, color.r, color.g, color.b,
                cell.m_position.x + 1, cell.m_position.y, 0.0f, color.r, color.g, color.b,
                cell.m_position.x, cell.m_position.y + 1, 0.0f, color.r, color.g, color.b,
                cell.m_position.x + 1, cell.m_position.y + 1, 0.0f, color.r, color.g, color.b
                });
        }

        // Create a single vertex array and buffer
        GLuint VAO, VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        // Bind the vertex array
        glBindVertexArray(VAO);

        // Bind and fill the buffer with the vertices data
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        // Set up vertex attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Render all cells in one pass
        paperShader.bind();
        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvp));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices.size() / 6);

        // Clean up
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);

        //glDisable(GL_DEPTH_TEST);

        //circleShader.bind();
        //{   // Draw circle around cursor
        //    glViewport(0, 0, window.getWindowSize().x, window.getWindowSize().y);
        //    vaoCircle.Bind();

        //    glm::vec3 colorCircle(0.0, 0.0, 0.0);
        //    {
        //        glUniform3fv(2, 1, glm::value_ptr(colorCircle));
        //        glUniform2fv(3, 1, glm::value_ptr(cursorPos));
        //        glDrawArrays(GL_LINE_STRIP, 0, cursorCircle.vertices.size() / 3);
        //    }
        //}
        ////Re-enable depth testing
        //glEnable(GL_DEPTH_TEST);

        glfwPollEvents();

        // Present result to the screen.
        window.swapBuffers();
    }

    return 0;
}
