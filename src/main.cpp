#include "cell.h"
#include "circle.h"
#include "heightmap.h"
#include "staggered_grid.h"
#include "global_constants.h"
#include "move_water.h"
#include "move_pigment.h"
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


Texture* texturePaper = NULL;

bool cursorCircle = true;
bool paperPlane = true;

//glm::vec3 point1(1.0, 1.0, 1.0);
//glm::vec3 point2(2.0, 2.0, 2.0);
//
//float verticesLine[] = {
//    point1.x, point1.y,
//    point2.x, point2.y
//};
//unsigned int indicesLine[] = {
//0, 1 // Indices to define the line segment using vertices array
//};



// Two triangles:
float verticesPlane[] = {
    // positions          // texture coords
    -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, // bottom left
     1.0f, -1.0f, 0.0f,   1.0f, 0.0f, // bottom right
     1.0f,  1.0f, 0.0f,   1.0f, 1.0f, // top right
    -1.0f,  1.0f, 0.0f,   0.0f, 1.0f  // top left
};

unsigned int indicesPlane[] = {
    0, 1, 2,  // first triangle
    2, 3, 0   // second triangle
};



int main()
{
    Window window{ "Watercolor", glm::ivec2(WIDTH, HEIGHT), OpenGLVersion::GL45 };
    float aspectRatio = window.getAspectRatio();
    window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);

    std::cout << "aspectratio: " << aspectRatio << "\n";

    Circle cursorCircle(&window, glm::vec2(0.0f), 0.1f, 4, 200); 

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
            Grid.push_back(Cell(glm::vec2(i, j), 1));
            Grid[i, j].m_height = heightmap[j][i];
        }
    }
    

    // Key handle function
    window.registerKeyCallback([&](int key, int /* scancode */, int action, int /* mods */) {
        if (action != GLFW_RELEASE)
            return;

        const bool shiftPressed = window.isKeyPressed(GLFW_KEY_LEFT_SHIFT) || window.isKeyPressed(GLFW_KEY_RIGHT_SHIFT);

        //switch (key) {
        //default:
        //    break;
        //};
    });

    const Shader circleShader = ShaderBuilder().addStage(GL_VERTEX_SHADER, "shaders/circle_vertex.glsl").addStage(GL_FRAGMENT_SHADER, "shaders/line_frag.glsl").build(); 
    const Shader planeShader = ShaderBuilder().addStage(GL_VERTEX_SHADER, "shaders/plane_vertex.glsl").addStage(GL_FRAGMENT_SHADER, "shaders/plane_frag.glsl").build();

    VertexBuffer vboPlane(verticesPlane, sizeof(verticesPlane)); 
    VertexBuffer iboPlane(indicesPlane, sizeof(indicesPlane));  
    VertexArray vaoPlane; 
    vaoPlane.Bind();
    vaoPlane.AddIndices(vaoPlane, iboPlane);
    vaoPlane.AddBuffer(vboPlane, 0, 3, GL_FLOAT, 5 * sizeof(float), (void*)0); // Add positions attribute
    vaoPlane.AddBuffer(vboPlane, 1, 2, GL_FLOAT, 5 * sizeof(float), (void*)(3 * sizeof(float))); // add texture attribute  
    vaoPlane.Unbind(); 
    // Create a texture on the GPU with 3 channels with 8 bits each.
    texturePaper = new Texture(GL_TEXTURE_2D, "resources/Watercolor_paper_texture.jpeg");
    if (!texturePaper->Load()) {
        return 1;
    }
    
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
        glm::vec2 cursorPos = window.getCursorPos();
        cursorPos = (cursorPos - glm::vec2(window.getWindowSize()) * 0.5f) / (glm::vec2(window.getWindowSize()) * 0.5f);

        cursorCircle.updateCenter(cursorPos);

        glViewport(0, 0, window.getWindowSize().x, window.getWindowSize().y);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // If LMB clicked -> m_pigmentConc of every cell within radius is 1.0
        if (window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
            for (int j = cursorPos.y - cursorCircle.m_radius; j <= cursorPos.y + cursorCircle.m_radius; j++) { 
                for (int i = cursorPos.x - cursorCircle.m_radius; i <= cursorPos.x + cursorCircle.m_radius; i++) {
                }
            }
        }

        planeShader.bind();
        GLuint gSamplerLocation = glGetUniformLocation(planeShader.getProgram(), "gSampler");
        {   // Draw paper plane
            glViewport(0, 0, window.getWindowSize().x, window.getWindowSize().y); 
            vaoPlane.Bind(); 
            glm::vec3 colorPlane(1.0, 1.0, 1.0); 
            { 
                texturePaper->Bind(GL_TEXTURE0); 
                
                glUniform1i(gSamplerLocation, 0);    
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
            } 
        }

        glDisable(GL_DEPTH_TEST);

        circleShader.bind();
        {   // Draw circle around cursor
            glViewport(0, 0, window.getWindowSize().x, window.getWindowSize().y); 
            vaoCircle.Bind(); 
            
            glm::vec3 colorCircle(0.0, 0.0, 0.0);
            {
                glUniform3fv(2, 1, glm::value_ptr(colorCircle));
                glUniform2fv(3, 1, glm::value_ptr(cursorPos)); 
                glDrawArrays(GL_LINE_STRIP, 0, cursorCircle.vertices.size() / 3);
            }
        }
        // Re-enable depth testing
        glEnable(GL_DEPTH_TEST); 

        /* Move water functions. */
        //UpdateVelocities(Grid, &x_velocity, &y_velocity, water_pressure);
        //RelaxDivergence(&x_velocity, &y_velocity, water_pressure);
        /* Here we would do flow outward if we are implementing that function */

        /* Pigment functions */
        //movePigment(Grid, &x_velocity, &y_velocity);

        glfwPollEvents();

        // Present result to the screen.
        window.swapBuffers();
    }

    return 0;
}
