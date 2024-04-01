#include "heightmap.h"



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

// Function to visualize the heightmap using OpenCV
void visualizeHeightmap(std::vector<std::vector<double>>& heightmap) 
{
    // Create an OpenCV Mat object to store the image
    cv::Mat image(heightmap.size(), heightmap[0].size(), CV_8UC1);

    // Iterate over each pixel in the heightmap
    for (int y = 0; y < heightmap.size(); ++y) {
        for (int x = 0; x < heightmap[y].size(); ++x) {
            // Convert the normalized height value to pixel intensity (0-255)
            int intensity = static_cast<int>(heightmap[y][x] * 255);

            // Set the pixel value in the image
            image.at<uchar>(y, x) = intensity;
        }
    }

    // Display the image using OpenCV
    cv::imshow("Heightmap", image);

    // Save the image as a PNG file
    std::string filename = "resources/heightmap.png";
    bool success = cv::imwrite(filename, image);

    // Check if the image writing was successful
    if (success) {
        std::cout << "Heightmap image saved successfully." << std::endl;
    }
    else {
        std::cerr << "Error: Failed to save heightmap image." << std::endl;
    }

    cv::waitKey(0);
}

std::vector<VertexColor> createHeightmapVertices(std::vector<std::vector<double>>& heightmap) 
{
    std::vector<VertexColor> vertices; 
    unsigned int width = heightmap[0].size(); 
    unsigned int height = heightmap.size();  

    for (unsigned int i = 0; i < height; i++)
    {
        for (unsigned int j = 0; j < width; j++)
        {
            // Get height at coordinate (i, j)
            double heightValue = heightmap[i][j]; 

            VertexColor vertex; 
            // Calculate vertex position based on height and grid position
            vertex.position.x = static_cast<float>(j);  // v.x 
            vertex.position.y = static_cast<float>(heightValue * 10); // v.y
            vertex.position.z = static_cast<float>(i);   // v.z 

            vertices.push_back(vertex);
        }
    }
    
    return vertices;
}

std::vector<unsigned int> createHeightmapIndices(std::vector<std::vector<double>>& heightmap)
{
    // index generation
    std::vector<unsigned int> indices;
    unsigned int width = heightmap[0].size(); 
    unsigned int height = heightmap.size();  

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

void createNormals(std::vector<std::vector<double>>& heightmap, std::vector<VertexColor>& vertices, std::vector<unsigned int>& indices) 
{
    unsigned int width = heightmap[0].size();
    unsigned int height = heightmap.size();

    // Iterate over each vertex in the grid
    for (unsigned int i = 0; i < height - 1; ++i) {
        for (unsigned int j = 0; j < width - 1; ++j) {
            glm::vec3 normal(0.0f, 0.0f, 0.0f);

            // Indices of the current vertex and its neighboring vertices
            int v0Index = i * width + j;
            int v1Index = i * width + j + 1;
            int v2Index = (i + 1) * width + j;
            int v3Index = (i + 1) * width + j + 1;

            // Calculate normals for the upper triangle
            glm::vec3 edge1 = vertices[v1Index].position - vertices[v0Index].position; 
            glm::vec3 edge2 = vertices[v2Index].position - vertices[v0Index].position; 
            glm::vec3 normalUpper = glm::cross(edge1, edge2); 
            glm::normalize(normalUpper); 

            // Calculate normals for the lower triangle
            edge1 = vertices[v2Index].position - vertices[v1Index].position;
            edge2 = vertices[v3Index].position - vertices[v1Index].position; 
            glm::vec3 normalLower = glm::cross(edge1, edge2);
            glm::normalize(normalLower); 

            // Assign normals to the vertices 
            vertices[v0Index].normal = normalUpper; 
            //vertices[v1Index].normal = normalUpper; 
            //vertices[v2Index].normal = normalUpper; 

            //vertices[v1Index].normal = normalLower; 
            //vertices[v2Index].normal = normalLower; 
            //vertices[v3Index].normal = normalLower; 

            //std::cout << "vertices.normal: (" << normalUpper.x << "," << normalUpper.y << "," << normalUpper.z << ")\n";
            //std::cout << "vertices[v1Index].normal: (" << vertices[v1Index].normal.x << "," << vertices[v1Index].normal.y << "," << vertices[v1Index].normal.z << ")\n";
            //std::cout << "vertices[v2Index].normal: (" << vertices[v2Index].normal.x << "," << vertices[v2Index].normal.y << "," << vertices[v2Index].normal.z << ")\n";
            //std::cout << "vertices[v3Index].normal: (" << vertices[v3Index].normal.x << "," << vertices[v3Index].normal.y << "," << vertices[v3Index].normal.z << ")\n";
        }
    }
}

void drawHeightmap(std::vector<std::vector<double>>& heightmap, std::vector<float>& vertices, std::vector<unsigned int>& indices, const glm::mat4 mvp, const Shader& shader) 
{ 
    unsigned int width = heightmap[0].size(); 
    unsigned int height = heightmap.size();   

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
    shader.bind(); 
    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(mvp)); 
    glBindVertexArray(terrainVAO); 
    // render the mesh triangle strip by triangle strip - each row at a time
    for (unsigned int strip = 0; strip < NUM_STRIPS; ++strip)
    {
        glDrawElements(GL_TRIANGLE_STRIP,   // primitive type
            NUM_VERTS_PER_STRIP, // number of indices to render
            GL_UNSIGNED_INT,     // index data type
            (void*)(sizeof(unsigned int) * NUM_VERTS_PER_STRIP * strip)); // offset to starting index 
    }
}