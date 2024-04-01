#pragma once
#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include <vector>  // Include necessary headers
#include <iostream>
#include <filesystem>
#include "heightmap.h"
//#include <opencv2/opencv.hpp>    
#include <framework/SimplexNoise.h> 
#include <stb/stb_image.h>
#include <framework/window.h>
#include <framework/shader.h>
#include <framework/mesh.h>
#include <glm/gtc/type_ptr.hpp> 

struct VertexColor {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;

    [[nodiscard]] constexpr bool operator==(const VertexColor&) const noexcept = default;
};
 
// Declare functions defined in heightmap.cpp
std::vector<std::vector<double>> generatePerlinNoise(int width, int height, int octaves, double lucanarity, double gain);
void normalizeHeightmap(std::vector<std::vector<double>>& heightmap);
void visualizeHeightmap(std::vector<std::vector<double>>& heightmap);

std::vector<VertexColor> createHeightmapVertices(std::vector<std::vector<double>>& heightmap);   
std::vector<unsigned int> createHeightmapIndices(std::vector<std::vector<double>>& heightmap);
void createNormals(std::vector<std::vector<double>>& heightmap, std::vector<VertexColor>& vertices, std::vector<unsigned int>& indices);
void drawHeightmap(std::vector<std::vector<double>>& heightmap, std::vector<float>& vertices, std::vector<unsigned int>& indices, const glm::mat4 mvp, const Shader& shader);

#endif // HEIGHTMAP_H 