#include <mcc/gl/vertex_buffer.hpp>

#include <GL/glew.h>

using namespace mcc;
using namespace mcc::gl;

Result<VertexBuffer, std::string> VertexBuffer::create(size_t size, const void* data, Usage usage) {
    GLuint vbo;
    GLenum gl_usage;

    if (usage == Usage::Static) {
        gl_usage = GL_STATIC_DRAW;
    } else if (usage == Usage::Dynamic) {
        gl_usage = GL_DYNAMIC_DRAW;
    } else if (usage == Usage::Stream) {
        gl_usage = GL_STREAM_DRAW;
    } else {
        std::abort(); // Unreachable code
    }

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, data, gl_usage);
    
    auto err = glGetError();
    if (err != 0) {
        return Result<VertexBuffer, std::string>::error("VertexBuffer::create() failed:\nglGetError() returned " + std::to_string(err));
    }

    return Result<VertexBuffer, std::string>::success(std::move(VertexBuffer(vbo)));
}

VertexBuffer::VertexBuffer(unsigned int vbo) :
    vbo(vbo) {
    // Empty
}

VertexBuffer::VertexBuffer(VertexBuffer&& rhs) {
    this->vbo = rhs.vbo;
    rhs.vbo = 0;
}

VertexBuffer::~VertexBuffer() {
    if (this->vbo != 0) {
        glDeleteBuffers(1, &this->vbo);
    }
}