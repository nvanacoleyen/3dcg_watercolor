#include "camera.h"
#include "cell.h"
#include "circle.h"
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
#include <iostream>
#include <span>
#include <vector>
#include <memory> 

// Configuration
constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;

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

std::vector<float> buildCircle(const glm::vec2 center, float radius, int thickness, int vCount, float aspectRatio)
{
    std::vector<float> verticesCircle;

    float angleIncrement = 2.0f * glm::pi<float>() / vCount;
    for (int t = 0; t < thickness; t++)
    {
        //    // Calculate the effective radius for the current layer
        float layerRadius = radius - (t / (WIDTH / 2.0f) ); // -(t / 100);  

        // positions
        for (int i = 0; i <= vCount; i++)
        {
            float currentAngle = angleIncrement * i;
            float x = layerRadius * cos(currentAngle) + center.x;
            float y = (layerRadius * sin(currentAngle) + center.y) * aspectRatio;
            verticesCircle.push_back(x);
            verticesCircle.push_back(y);  
            verticesCircle.push_back(0.0f); 
        }
    }
    return verticesCircle;
}


int main()
{
    Window window{ "Watercolor", glm::ivec2(WIDTH, HEIGHT), OpenGLVersion::GL45 };
    float aspectRatio = window.getAspectRatio();

    std::cout << "aspectratio: " << aspectRatio << "\n";

    Circle cursorCircle(&window, glm::vec2(0.0f), 0.1f, 4, 200); 

    // Create grid of cells
    std::vector<float> pigmentConcValues;
    std::vector<std::shared_ptr<Cell>> Grid;
    for (int j = 0; j < HEIGHT; j++) {
        for (int i = 0; i < WIDTH; i++) {
            Grid.push_back(std::make_shared<Cell>(glm::vec2(i, j), 1));
            pigmentConcValues.push_back(Grid[WIDTH * j + i]->m_pigmentConc);
        }
    }
    

    // Key handle function
    window.registerKeyCallback([&](int key, int /* scancode */, int action, int /* mods */) {
        if (action != GLFW_RELEASE)
            return;

        const bool shiftPressed = window.isKeyPressed(GLFW_KEY_LEFT_SHIFT) || window.isKeyPressed(GLFW_KEY_RIGHT_SHIFT);

        switch (key) {
        default:
            break;
        };
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
    GLuint texPaper;
    glGenTextures(1, &texPaper);
    glBindTexture(GL_TEXTURE_2D, texPaper);   
    // Set behavior for when texture coordinates are outside the [0, 1] range.
    glTextureParameteri(texPaper, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    glTextureParameteri(texPaper, GL_TEXTURE_WRAP_T, GL_REPEAT); 
    // Set interpolation for texture sampling (GL_NEAREST for no interpolation).
    glTextureParameteri(texPaper, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    glTextureParameteri(texPaper, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    //// Plane TEXTURES
    int widthPaper, heightPaper, sourceNumChannels; // Number of channels in source image. pixels will always be the requested number of channels (3). 
    stbi_uc* paperTexture = stbi_load("resources/Watercolor_paper_texture.jpeg", &widthPaper, &heightPaper, &sourceNumChannels, STBI_rgb);
    if (paperTexture) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, widthPaper, heightPaper, 0, GL_RGB, GL_UNSIGNED_BYTE, paperTexture);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
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
            for (int j = 0; j < cursorCircle.m_radius; j++) { 
                for (int i = 0; i < cursorCircle.m_radius; i++) { 
                    Grid[WIDTH * j + i]->m_pigmentConc = 1.0; 
                }
            }
        }

        planeShader.bind();
        {   // Draw paper plane
            glViewport(0, 0, window.getWindowSize().x, window.getWindowSize().y); 
            vaoPlane.Bind(); 
            glm::vec3 colorPlane(1.0, 1.0, 1.0); 
            {
                glActiveTexture(GL_TEXTURE0); 
                glBindTexture(GL_TEXTURE_2D, texPaper);  
                glUniform1i(4, 0);   
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

        glfwPollEvents();

        // Present result to the screen.
        window.swapBuffers();
    }

    return 0;
}
