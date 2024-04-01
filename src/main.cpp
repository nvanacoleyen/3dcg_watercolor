#include "cell.h"
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


struct Light {
    glm::vec3 position;
    glm::vec3 color;
};
std::vector lights{ Light { glm::vec3(0, 0, 3), glm::vec3(1) } };

bool calculate_watercolour = false;
float brush_radius = 7;

int main()
{
    Window window{ "Watercolor", glm::ivec2(WIDTH, HEIGHT), OpenGLVersion::GL45 };
    Camera camera{ &window, glm::vec3(393.572052f, 290.958832f, 737.367920f), glm::vec3(0.00720430166f, 0.0117728803f, -0.999904811f) };
    constexpr float fov = glm::pi<float>() / 4.0f;
    constexpr float aspect = static_cast<float>(WIDTH) / static_cast<float>(HEIGHT);
    const glm::mat4 mainProjectionMatrix = glm::perspective(fov, aspect, 0.1f, 1000.0f);
    float aspectRatio = window.getAspectRatio();
    window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);


    /* GENERATE HEIGHTMAP */
    int octaves = 5;
    double lucanarity = 2.1042;
    double gain = 3.0;
    std::vector<std::vector<double>> heightmap = generatePerlinNoise(WIDTH, HEIGHT, octaves, lucanarity, gain);  
    // Normalize heightmap values to range [0, 1]
    normalizeHeightmap(heightmap);  
    // Create heightmap vertices, indices and normals
    std::vector<VertexColor> verticesTerrain = createHeightmapVertices(heightmap);   
    std::vector<unsigned int> indicesTerrain = createHeightmapIndices(heightmap);   
    createNormals(heightmap, verticesTerrain, indicesTerrain); 

    /* GENERATE GRID OF CELLS */
    Staggered_Grid x_velocity(WIDTH, HEIGHT, true); 
    Staggered_Grid y_velocity(WIDTH, HEIGHT, false);
    std::vector<float> water_pressure(WIDTH * HEIGHT, 0.f);
    // Create grid of cells
    std::vector<Cell> Grid;
    for (int j = 0; j < HEIGHT; j++) {
        for (int i = 0; i < WIDTH; i++) {
            Grid.push_back(Cell(glm::vec3(i, j, heightmap[j][i] * 100), 1)); 
            Grid[WIDTH * j + i].m_height = heightmap[j][i];
        }
    }

    /* RENDER GRID */
    std::vector<float> vertices;
    for (int i = 0; i < Grid.size(); i++) 
    {
        glm::vec3 color;
        // Color with/without water/pigment concentration
        if (Grid[i].m_waterConc == 1 && Grid[i].m_pigmentConc == 0) {
            color = normalize(glm::vec3(191, 64, 191));
        }
        else if (Grid[i].m_pigmentConc != 0) {
            color = glm::vec3(Grid[i].m_pigmentConc, 0, 0);
        }
        else {
            color = glm::vec3(1.0, 0.95, 0.9);
        } // Create terrain vertices
        vertices.insert(vertices.end(), {
            verticesTerrain[i].position.x,      verticesTerrain[i].position.z,      verticesTerrain[i].position.y,
            verticesTerrain[i].normal.x,        verticesTerrain[i].normal.z,        verticesTerrain[i].normal.y,
            color.r,                            color.g,                            color.b,
            verticesTerrain[i].position.x + 1,  verticesTerrain[i].position.z,      verticesTerrain[i].position.y,
            verticesTerrain[i].normal.x + 1,    verticesTerrain[i].normal.z,        verticesTerrain[i].normal.y,
            color.r,                            color.g,                            color.b,
            verticesTerrain[i].position.x,      verticesTerrain[i].position.z + 1,  verticesTerrain[i].position.y,
            verticesTerrain[i].normal.x,        verticesTerrain[i].normal.z + 1,    verticesTerrain[i].normal.y,
            color.r,                            color.g,                            color.b,
            verticesTerrain[i].position.x + 1,  verticesTerrain[i].position.z + 1,  verticesTerrain[i].position.y,
            verticesTerrain[i].normal.x + 1,    verticesTerrain[i].normal.z + 1,    verticesTerrain[i].normal.y,
            color.r,                            color.g,                            color.b
            });
    }

    std::cout << "vertices.size(): " << vertices.size() << "\n";

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
            if (brush_radius < 20) {
                brush_radius += 1;
            }
            break;
        default:
            break;
        };
    });

    bool isDragging = false;

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

    window.registerMouseMoveCallback([&](const glm::vec2& cursorPos) {
        if (isDragging) {
            for (int j = cursorPos.y - brush_radius; j <= cursorPos.y + brush_radius; j++) {
                for (int i = cursorPos.x - brush_radius; i <= cursorPos.x + brush_radius; i++) {
                    if (j < 600 && i < 800 && j >= 0 && i >= 0) {
                        float dist = sqrt(pow(i - cursorPos.x, 2) + pow(j - cursorPos.y, 2));
                        if (dist <= brush_radius) {
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
            }
        }
    });

    /* SHADERS */
    const Shader paperShader = ShaderBuilder().addStage(GL_VERTEX_SHADER, "shaders/paper_vertex.glsl").addStage(GL_FRAGMENT_SHADER, "shaders/paper_frag.glsl").build();

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);


    // Main loop
    while (!window.shouldClose()) {
        window.updateInput();
        camera.updateInput();

        //glViewport(0, 0, window.getWindowSize().x, window.getWindowSize().y);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Enable color writes. 
        
        glm::vec2 cursorPos = window.getCursorPos() / window.getDpiScalingFactor();

        if (isDragging == true) {
            // For every square in the terrain:
            for (size_t i = 0; i < vertices.size(); i += 36)
            {   // For every vertex in the square:
                for (size_t k = 0; k < 4; k++)
                {
                    // Extract vertex position from buffer
                    glm::vec2 vertexPos = glm::vec2(vertices[i + k * 9], vertices[i + k * 9 + 1]);

                    // Calculate distance between cursor position and vertex position
                    float distance = glm::distance(cursorPos, vertexPos);

                    // If the vertex is close to the cursor position, update its color
                    if (distance < 20) {
                        // Update color attribute of the vertex
                        vertices[i + k * 9 + 6] = 0.93;
                        vertices[i + k * 9 + 7] = 0.05; 
                        vertices[i + k * 9 + 8] = 0.8;
                    }
                }
            }
            // Update the buffer data 
            glBindBuffer(GL_ARRAY_BUFFER, VBO); 
            glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data()); 
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

        paperShader.bind();
        {
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvp));
            //glUniform3fv(1, 1, glm::value_ptr(lights[0].position));  
            glBindVertexArray(VAO);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices.size() / 9);
        }

        glfwPollEvents();

        // Present result to the screen.
        window.swapBuffers();
    }
    // Clean up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    return 0;
}
