#include "terrain.h"

Terrain::Terrain(int octaves, double lucanarity, double gain, int depth, int width, double minHeight, double maxHeight)
    : m_depth(depth), m_width(width)
{
    m_heightMap = std::vector<std::vector<double>>(depth, std::vector<double>(width)); 
    GLuint m_vao, m_vbo, m_ibo; 

    generateHeightmap(octaves, lucanarity, gain, minHeight, maxHeight);
    createVertices(); 
    createTriangles();
    calculateNormals();   
};

void Terrain::generateHeightmap(int octaves, double lucanarity, double gain, double minHeight, double maxHeight)
{
    SimplexNoise noise;

    // Generate simplex noise values for each point in a 2D grid
    for (std::size_t y = 0; y < m_depth; ++y) {
        for (std::size_t x = 0; x < m_width; ++x) {
            double xPos = double(x) / double(m_width) - 0.5;
            double yPos = double(y) / double(m_depth) - 0.5;

            // Get height at (xPos, yPos) from Simplex noise
            double currentHeight = noise.unsignedFBM(xPos, yPos, octaves, lucanarity, gain);

            m_heightMap[y][x]  = currentHeight;

        }
    }

    normalizeHeightmap(minHeight, maxHeight);  
}

void Terrain::normalizeHeightmap(double minRange, double maxRange) { 
    double min_height = DBL_MAX;
    double max_height = -DBL_MAX;

    for (const auto& row : m_heightMap) {
        for (double height : row) {
            if (height < min_height) min_height = height;
            if (height > max_height) max_height = height;
        }
    }
    //std::cout << "max_height: " << max_height << ", min_height: " << min_height << "\n";
    double height_delta = max_height - min_height;
    double height_range = maxRange - minRange; 

    for (auto& row : m_heightMap) { 
        for (double& height : row) {
            height = (height - min_height) / height_delta * height_range + minRange; 
        }
    }
}

void Terrain::createVertices()
{
    for (unsigned int y = 0; y < m_depth; y++) {
        for (unsigned int x = 0; x < m_width; x++) {
            // Get height at coordinate (i, j)
            double heightValue = m_heightMap[y][x]; 
             
            paperVertex vertex; 

            // Calculate vertex position based on height and grid position
            vertex.position = glm::vec3(x, y, heightValue);  

            // Give paper initial color
            vertex.color = glm::vec3(1.0f);  

            m_mesh.vertices.push_back(vertex);  
        }
    }
}

void Terrain::createTriangles()
{
    // Get indices of the 2 triangles per cell
    for (unsigned int y = 0; y < m_depth - 1; y++) {
        for (unsigned int x = 0; x < m_width - 1; x++) {
            unsigned int indexBottomLeft = y * m_width + x;
            unsigned int indexTopLeft = (y + 1) * m_width + x;
            unsigned int indexTopRight = (y + 1) * m_width + x + 1;
            unsigned int indexBottomRight = y * m_width + x + 1;

            // Add top left triangle
            glm::uvec3 triangle1(indexBottomLeft, indexTopLeft, indexTopRight); 
            m_mesh.triangles.push_back(triangle1);

            // Add bottom right triangle
            glm::uvec3 triangle2(indexBottomLeft, indexTopRight, indexBottomRight);
            m_mesh.triangles.push_back(triangle2);
        }
    }
}

void Terrain::calculateNormals()   
{
    std::vector<int> counters; // counts the normals of every vertex
    std::vector<glm::vec3> normals; // normals of every vertex
     
    counters.resize(m_mesh.vertices.size());
    normals.resize(m_mesh.vertices.size());
     
    for (glm::vec3 t : m_mesh.triangles) {
        // indices of three points in the triangle
        int i0 = t.x;
        int i1 = t.y;
        int i2 = t.z;

        // points in the triangle
        glm::vec3 pos0 = m_mesh.vertices[i0].position;
        glm::vec3 pos1 = m_mesh.vertices[i1].position;
        glm::vec3 pos2 = m_mesh.vertices[i2].position;

        // Calculate triangle normal
        glm::vec3 edge1 = pos1 - pos0;
        glm::vec3 edge2 = pos2 - pos0;
        glm::vec3 normal = glm::cross(edge1, edge2);
        normalize(normal);

        m_mesh.vertices[i0].normal = normal;
        m_mesh.vertices[i1].normal = normal; 
        m_mesh.vertices[i2].normal = normal;  

        //std::cout << "i0: " << i0 << ", i1: " << i1 << ", i2: " << i2 << "\n";
        //std::cout << "pos0: (" << pos0.x << ", " << pos0.y << ", " << pos0.z << ")\n";
        //std::cout << "pos1: (" << pos1.x << ", " << pos1.y << ", " << pos1.z << ")\n";
        //std::cout << "pos2: (" << pos2.x << ", " << pos2.y << ", " << pos2.z << ")\n";
        //std::cout << "normal: (" << normal.x << ", " << normal.y << ", " << normal.z << ")\n";

        normals[i0] += normal;
        normals[i1] += normal; 
        normals[i2] += normal; 
    
        counters[i0]++; 
        counters[i1]++; 
        counters[i2]++; 
    }
    
    for (int i = 0; i < static_cast<int>(normals.size()); ++i) {
        if (counters[i] > 0)
            normals[i] /= counters[i];
        else
            glm::normalize(normals[i]);
        m_mesh.vertices[i].normal = normals[i];
    }
}

void Terrain::createGLState()
{
    glCreateBuffers(1, &m_ibo);
    glNamedBufferData(m_ibo, static_cast<GLsizeiptr>(m_mesh.triangles.size() * sizeof(decltype(paperMesh::triangles)::value_type)), m_mesh.triangles.data(), GL_STATIC_DRAW);

    glCreateBuffers(1, &m_vbo);
    glNamedBufferData(m_vbo, static_cast<GLsizeiptr>(m_mesh.vertices.size() * sizeof(paperVertex)), m_mesh.vertices.data(), GL_DYNAMIC_DRAW);

    glCreateVertexArrays(1, &m_vao);
    glVertexArrayElementBuffer(m_vao, m_ibo);

    glVertexArrayVertexBuffer(m_vao, 0, m_vbo, offsetof(paperVertex, position), sizeof(paperVertex));
    glVertexArrayVertexBuffer(m_vao, 1, m_vbo, offsetof(paperVertex, normal), sizeof(paperVertex)); 
    glVertexArrayVertexBuffer(m_vao, 2, m_vbo, offsetof(paperVertex, color), sizeof(paperVertex));
    glEnableVertexArrayAttrib(m_vao, 0);
    glEnableVertexArrayAttrib(m_vao, 1); 
    glEnableVertexArrayAttrib(m_vao, 2);
}

void Terrain::updateBuffer()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_mesh.vertices.size() * sizeof(paperVertex), m_mesh.vertices.data());  
    glBindBuffer(GL_ARRAY_BUFFER, 0);  
}

void Terrain::Render()
{
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, (m_depth - 1) * (m_width - 1) * 6, GL_UNSIGNED_INT, NULL);

    glBindVertexArray(0); 
}

paperMesh &Terrain::getMesh()   
{
    return m_mesh;
}

std::vector<std::vector<double>> &Terrain::getHeightmap()  
{
    return m_heightMap;   
}

GLuint &Terrain::getVAO()
{
    return m_vao;
}

GLuint &Terrain::getVBO() 
{
    return m_vbo;
}
GLuint &Terrain::getIBO()
{
    return m_ibo;
}