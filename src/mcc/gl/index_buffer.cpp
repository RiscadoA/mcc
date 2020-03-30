#include <mcc/gl/index_buffer.hpp>

#include <GL/glew.h>

using namespace mcc;
using namespace mcc::gl;

Result<IndexBuffer, std::string> IndexBuffer::create(size_t size, const void* data, Usage usage) {
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
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, gl_usage);
    
    auto err = glGetError();
    if (err != 0) {
        return Result<IndexBuffer, std::string>::error("IndexBuffer::create() failed:\nglGetError() returned " + std::to_string(err));
    }

    return Result<IndexBuffer, std::string>::success(std::move(IndexBuffer(vbo)));
}

IndexBuffer::IndexBuffer(unsigned int ibo) :
    ibo(ibo) {
    // Empty
}

IndexBuffer::IndexBuffer(IndexBuffer&& rhs) {
    this->ibo = rhs.ibo;
    rhs.ibo = 0;
}

IndexBuffer& IndexBuffer::operator=(IndexBuffer&& rhs) {
    if (this->ibo != 0) {
        glDeleteBuffers(1, &this->ibo);
    }

    this->ibo = rhs.ibo;
    rhs.ibo = 0;
    
    return *this;
}

IndexBuffer::~IndexBuffer() {
    if (this->ibo != 0) {
        glDeleteBuffers(1, &this->ibo);
    }
}

Result<void*, std::string> IndexBuffer::map() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo);
    auto ret = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

    if (ret == nullptr) {
        return Result<void*, std::string>::error(
            "IndexBuffer::map() failed:\nglMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY) returned nullptr:\n"
            "glGetError() returned '" + std::to_string(glGetError()) + "'"
        );
    }

    return Result<void*, std::string>::success(std::move(ret));
}

void IndexBuffer::unmap() {
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
}

Result<void, std::string> IndexBuffer::update(size_t offset, size_t size, const void* data) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, GLintptr(offset), GLsizeiptr(size), data);

    auto err = glGetError();
    if (err != 0) {
        return Result<void, std::string>::error(
            "IndexBuffer::update() failed:\n"
            "glGetError() returned '" + std::to_string(err) + "'"
        );
    }

    return Result<void, std::string>::success();
}

void IndexBuffer::bind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ibo);
}
