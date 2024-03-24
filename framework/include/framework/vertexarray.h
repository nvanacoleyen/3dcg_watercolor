#pragma once
#ifndef VERTEXARRAY_H
#define VERTEXARRAY_H

#include "vertexbuffer.h"

class VertexArray {
public:
    VertexArray();
    ~VertexArray();
    void Bind() const;
    void Unbind() const;
    void AddBuffer(const VertexBuffer& vb, GLuint index, GLint size, GLenum type, GLsizei stride, const void* offset=NULL);
    void AddIndices(const VertexArray& vao, const VertexBuffer& vbo);

    GLuint GetID() const;

private:
    GLuint m_RendererID;
};

#endif // VERTEXARRAY_H 