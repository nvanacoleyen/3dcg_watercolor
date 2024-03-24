#include "vertexarray.h"

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &m_RendererID);
    glBindVertexArray(m_RendererID);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &m_RendererID);
}

void VertexArray::Bind() const
{
    glBindVertexArray(m_RendererID);
}

void VertexArray::Unbind() const
{
    glBindVertexArray(0);
}

void VertexArray::AddBuffer(const VertexBuffer& vb, GLuint index, GLint size, GLenum type, GLsizei stride, const void* offset)
{
    vb.Bind();
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, size, type, GL_FALSE, stride, offset);
} 

GLuint VertexArray::GetID() const
{
    return m_RendererID;
}

void VertexArray::AddIndices(const VertexArray& vao, const VertexBuffer& ibo)
{
    GLuint vaoID = vao.GetID();
    GLuint iboID = ibo.GetID(); 
    glVertexArrayElementBuffer(vaoID, iboID);
}