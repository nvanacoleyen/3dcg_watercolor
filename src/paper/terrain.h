#ifndef TERRAIN_H 
#define TERRAIN_H

#include <vector>
#include <iostream>
#include <framework/mesh.h>
#include <framework/window.h>
#include <framework/SimplexNoise.h>
#include <framework/disable_all_warnings.h>
 
struct paperVertex {  
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	//glm::vec2 texCoord; // Texture coordinate

	[[nodiscard]] constexpr bool operator==(const paperVertex&) const noexcept = default; 
};

struct paperMaterial { 
	glm::vec3 kd; // Diffuse color.
	glm::vec3 ks{ 0.0f };
	float shininess{ 1.0f };
	float transparency{ 1.0f };

	// Optional texture that replaces kd; use as follows:
	// 
	// if (material.kdTexture) {
	//   material.kdTexture->getTexel(...);
	// }
	std::shared_ptr<Image> kdTexture;
};

struct paperMesh { 
	// Vertices contain the vertex positions and normals of the mesh.
	std::vector<paperVertex> vertices; 
	// A triangle contains a triplet of values corresponding to the indices of the 3 vertices in the vertices array.
	std::vector<glm::uvec3> triangles;

	Material material;
};

class Terrain
{
public:
	Terrain(int octaves, double lucanarity, double gain, int depth, int width, double minHeight, double maxHeight);

	void generateHeightmap(int octaves, double lucanarity, double gain, double minHeight, double maxHeight);
	void normalizeHeightmap(double minHeight, double maxHeight); 

	void createVertices();  
	void createTriangles();

	void calculateNormals();  

	void createGLState(); 
	void updateBuffer();
	void Render(); 

	paperMesh &getMesh();  
	std::vector<std::vector<double>> &getHeightmap();
	GLuint &getVAO(); 
	GLuint &getVBO();
	GLuint &getIBO();

//private:
	std::vector<std::vector<double>> m_heightMap;  
	paperMesh m_mesh;   

	int m_width = 0;
	int m_depth = 0;
	double m_min = 0;
	double m_max = 0;

	GLuint m_vao;
	GLuint m_vbo;
	GLuint m_ibo;
};
#endif