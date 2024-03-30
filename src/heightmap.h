#pragma once
#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

#include <vector>  // Include necessary headers

// Declare functions defined in heightmap.cpp
std::vector<std::vector<double>> generatePerlinNoise(int width, int height, int octaves, double lucanarity, double gain);
void normalizeHeightmap(std::vector<std::vector<double>>& heightmap);
void visualizeHeightmap(std::vector<std::vector<double>>& heightmap);

std::vector<float> createHeightmapVertices(const char* imagePath);
std::vector<unsigned int> createHeightmapIndices(const char* imagePath);
void drawHeightmap(const char* imagePath, std::vector<float> vertices, std::vector<unsigned int> indices);

#endif // HEIGHTMAP_H