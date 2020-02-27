#include <mcc/gl/shader.hpp>

#include <GL/glew.h>

using namespace mcc;
using namespace mcc::gl;

Result<Shader, std::string> Shader::create(const char* vs_src, const char* fs_src) {
    GLuint program, vs, fs;
    GLint status;

    vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_src, nullptr);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    if (!status) {
        std::string info_log;
        glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &status);
        info_log.resize(status);
        glGetShaderInfoLog(vs, GLsizei(info_log.size()) + 1, nullptr, &info_log[0]);
        info_log = "Shader::create() failed:\nglCreateShader(GL_VERTEX_SHADER) failed:\n" + info_log;
        return Result<Shader, std::string>::error(std::move(info_log));
    }

    fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_src, nullptr);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    if (!status) {
        std::string info_log;
        glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &status);
        info_log.resize(status);
        glGetShaderInfoLog(fs, GLsizei(info_log.size()) + 1, nullptr, &info_log[0]);
        info_log = "Shader::create() failed:\nglCreateShader(GL_FRAGMENT_SHADER) failed:\n" + info_log;
        return Result<Shader, std::string>::error(std::move(info_log));
    }

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status) {
        std::string info_log;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &status);
        info_log.resize(status);
        glGetProgramInfoLog(program, GLsizei(info_log.size()) + 1, nullptr, &info_log[0]);
        info_log = "Shader::create() failed:\nglLinkProgram() failed:\n" + info_log;
        return Result<Shader, std::string>::error(std::move(info_log));
    }

    return Result<Shader, std::string>::success(std::move(Shader(program, vs, fs)));
}

Shader::Shader(unsigned int program, unsigned int vs, unsigned int fs) :
    program(program), vs(vs), fs(fs) {
    // Empty
}

Shader::Shader(Shader&& rhs) {
    this->program = rhs.program;
    this->vs = rhs.vs;
    this->fs = rhs.fs;
    rhs.program = 0;
    rhs.vs = 0;
    rhs.fs = 0;
}

Shader::~Shader() {
    if (this->program != 0) {
        glDeleteProgram(this->program);
    }

    if (this->vs != 0) {
        glDeleteShader(this->vs);
    }

    if (this->fs != 0) {
        glDeleteShader(this->fs);
    }
}

void Shader::bind() {
    glUseProgram(this->program);
}

Result<unsigned int, std::string> Shader::get_attribute_location(const char* name) {
    auto location = glGetAttribLocation(this->program, name);
    if (location < 0) {
        return Result<unsigned int, std::string>::error("Shader::get_attribute_location() failed:\nNo attribute '" + std::string(name) + "' found");
    }
    return Result<unsigned int, std::string>::success(unsigned int(location));
}

Result<unsigned int, std::string> Shader::get_uniform_location(const char* name) {
    auto location = glGetUniformLocation(this->program, name);
    if (location < 0) {
        return Result<unsigned int, std::string>::error("Shader::get_uniform_location() failed:\nNo uniform '" + std::string(name) + "' found");
    }
    return Result<unsigned int, std::string>::success(unsigned int(location));
}