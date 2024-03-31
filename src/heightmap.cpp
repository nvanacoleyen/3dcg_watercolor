#include "heightmap.h"
//#include <opencv2/opencv.hpp>  
#include <framework/SimplexNoise.h> 
#include <stb/stb_image.h>
#include <framework/window.h>
#include <framework/mesh.h>


std::vector<std::vector<double>> generatePerlinNoise(int width, int height, int octaves, double lucanarity, double gain)
{
    std::vector<std::vector<double>> heightmap(height, std::vector<double>(width));

    SimplexNoise noise;

    // Generate simplex noise values for each point in a 2D grid
    for (std::size_t y = 0; y < height; ++y) {
        for (std::size_t x = 0; x < width; ++x) {
            double xPos = double(x) / double(width) - 0.5;
            double yPos = double(y) / double(height) - 0.5;

            heightmap[y][x] = noise.signedFBM(xPos, yPos, octaves, lucanarity, gain);
        }
    }

    return heightmap;
}

void normalizeHeightmap(std::vector<std::vector<double>>& heightmap) {
    double min_height = DBL_MAX;
    double max_height = -DBL_MAX;

    for (const auto& row : heightmap) {
        for (double height : row) {
            if (height < min_height) min_height = height;
            if (height > max_height) max_height = height;
        }
    }

    double height_range = max_height - min_height;

    for (auto& row : heightmap) {
        for (double& height : row) {
            height = (height - min_height) / height_range;
        }
    }
}

//// Function to visualize the heightmap using OpenCV
//void visualizeHeightmap(std::vector<std::vector<double>>& heightmap) 
//{
//    // Create an OpenCV Mat object to store the image
//    cv::Mat image(heightmap.size(), heightmap[0].size(), CV_8UC1);
//
//    // Iterate over each pixel in the heightmap
//    for (int y = 0; y < heightmap.size(); ++y) {
//        for (int x = 0; x < heightmap[y].size(); ++x) {
//            // Convert the normalized height value to pixel intensity (0-255)
//            int intensity = static_cast<int>(heightmap[y][x] * 255);
//
//            // Set the pixel value in the image
//            image.at<uchar>(y, x) = intensity;
//        }
//    }
//
//    // Display the image using OpenCV
//    cv::imshow("Heightmap", image);
//
//    // Save the image as a PNG file
//    std::string filename = "resources/heightmap.png";
//    bool success = cv::imwrite(filename, image);
//
//    // Check if the image writing was successful
//    if (success) {
//        std::cout << "Heightmap image saved successfully." << std::endl;
//    }
//    else {
//        std::cerr << "Error: Failed to save heightmap image." << std::endl;
//    }
//
//    cv::waitKey(0);
//}

std::vector<float> createHeightmapVertices(std::string& imagePath)
{
    std::vector<float> vertices;

    // load height map texture
    int width, height, nChannels;
    unsigned char* data = stbi_load(imagePath.c_str(),
        &width, &height, &nChannels, 0); 
    if (stbi_failure_reason())
        std::cout << stbi_failure_reason();

    for (unsigned int i = 0; i < height; i++)
    {
        for (unsigned int j = 0; j < width; j++)
        {
            // retrieve texel for (i,j) tex coord
            unsigned char* texel = data + (j + width * i) * nChannels;
            // raw height at coordinate
            unsigned char y = texel[0];
        }
    }
    stbi_image_free(data);

    return vertices;
}

std::vector<unsigned int> createHeightmapIndices(std::string& imagePath)
{
    int width, height, nChannels;
    unsigned char* data = stbi_load(imagePath.c_str(),
        &width, &height, &nChannels, 0);

    if (stbi_failure_reason()) 
        std::cout << stbi_failure_reason(); 

    // index generation
    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < height - 1; i++)       // for each row a.k.a. each strip
    {
        for (unsigned int j = 0; j < width; j++)      // for each column
        {
            for (unsigned int k = 0; k < 2; k++)      // for each side of the strip
            {
                indices.push_back(j + width * (i + k));
            }
        }
    }

    return indices;
}

//void createMesh(std::vector<float> vertices, std::vector<unsigned int> indices)
//{
//
//}

void drawHeightmap(std::string& imagePath, std::vector<float> vertices, std::vector<unsigned int> indices)
{
    int width, height, nChannels;
    unsigned char* data = stbi_load(imagePath.c_str(),
        &width, &height, &nChannels,
        0);

    const unsigned int NUM_STRIPS = height - 1; 
    const unsigned int NUM_VERTS_PER_STRIP = width * 2; 

    // register VAO
    GLuint terrainVAO, terrainVBO, terrainEBO;
    glGenVertexArrays(1, &terrainVAO);
    glBindVertexArray(terrainVAO);

    glGenBuffers(1, &terrainVBO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),       // size of vertices buffer
        &vertices[0],                          // pointer to first element
        GL_STATIC_DRAW);

    glVertexArrayVertexBuffer(terrainVAO, 0, terrainVBO, offsetof(Vertex, position), sizeof(Vertex));
    glVertexArrayVertexBuffer(terrainVAO, 1, terrainVBO, offsetof(Vertex, normal), sizeof(Vertex));
    glEnableVertexArrayAttrib(terrainVAO, 0);
    glEnableVertexArrayAttrib(terrainVAO, 1);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &terrainEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(unsigned int), // size of indices buffer
        &indices[0],                           // pointer to first element
        GL_STATIC_DRAW);
    
    // draw mesh
    glBindVertexArray(terrainVAO);
    // render the mesh triangle strip by triangle strip - each row at a time
    for (unsigned int strip = 0; strip < NUM_STRIPS; ++strip)
    {
        glDrawElements(GL_TRIANGLE_STRIP,   // primitive type
            NUM_VERTS_PER_STRIP, // number of indices to render
            GL_UNSIGNED_INT,     // index data type
            (void*)(sizeof(unsigned int) * NUM_VERTS_PER_STRIP * strip)); // offset to starting index 
    }

    //VertexBuffer vboPlane(verticesPlane, sizeof(verticesPlane));
    //VertexBuffer iboPlane(indicesPlane, sizeof(indicesPlane));
    //VertexArray vaoPlane;
    //vaoPlane.Bind();
    //vaoPlane.AddIndices(vaoPlane, iboPlane);
    //vaoPlane.AddBuffer(vboPlane, 0, 3, GL_FLOAT, 5 * sizeof(float), (void*)0); // Add positions attribute
    //vaoPlane.AddBuffer(vboPlane, 1, 2, GL_FLOAT, 5 * sizeof(float), (void*)(3 * sizeof(float))); // add texture attribute  
    //vaoPlane.Unbind();
}