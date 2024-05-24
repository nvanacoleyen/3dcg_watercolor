#ifndef TRIANGLE_LIST_H
#define TRIANGLE_LIST_H

#include <vector>
#include <framework/window.h>

class Terrain;

class TriangleList {
public:
	TriangleList();



private:
	int m_width = 0;
	int m_depth = 0;
	GLuint m_vao;
	GLuint m_vbo;
	GLuint m_ibo;

};



#endif