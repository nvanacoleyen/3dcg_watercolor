#pragma once
#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

// Library for loading an image
#include <framework/window.h>

class VertexBuffer {
public:
    VertexBuffer(const void* data, size_t size);
    ~VertexBuffer();
    void Bind() const;
    void Update(const void* data, size_t size) const;
    void Unbind() const;

    GLuint GetID() const;      

private:
    GLuint m_RendererID;
};

#endif // VERTEXBUFFER_H 