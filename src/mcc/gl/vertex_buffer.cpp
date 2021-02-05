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
    if (this->vbo != 0) {
        glDeleteBuffers(1, &this->vbo);
    }

    this->vbo = rhs.vbo;
    rhs.vbo = 0;
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& rhs) {
    if (this->vbo != 0) {
        glDeleteBuffers(1, &this->vbo);
    }

    this->vbo = rhs.vbo;
    rhs.vbo = 0;
    
    return *this;
}

VertexBuffer::~VertexBuffer() {
    if (this->vbo != 0) {
        glDeleteBuffers(1, &this->vbo);
    }
}

Result<void*, std::string> VertexBuffer::map() {
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    auto ret = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

    if (ret == nullptr) {
        return Result<void*, std::string>::error(
            "VertexBuffer::map() failed:\nglMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY) returned nullptr:\n"
            "glGetError() returned '" + std::to_string(glGetError()) + "'"
        );
    }

    return Result<void*, std::string>::success(std::move(ret));
}

void VertexBuffer::unmap() {
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

void mcc::gl::VertexBuffer::bind() {
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
}

Result<void, std::string> VertexBuffer::update(size_t offset, size_t size, const void* data) {
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, GLintptr(offset), GLsizeiptr(size), data);

    auto err = glGetError();
    if (err != 0) {
        return Result<void, std::string>::error(
            "VertexBuffer::update() failed:\n"
            "glGetError() returned '" + std::to_string(err) + "'"
        );
    }

    return Result<void, std::string>::success();
}
