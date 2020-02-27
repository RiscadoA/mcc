#pragma once

#include <string>

#include <mcc/result.hpp>

namespace mcc::gl {
    class Shader final {
    public:
        static Result<Shader, std::string> create(const char* vs, const char* fs);
        
        Shader(Shader&& rhs);
        ~Shader();

        void bind();
        Result<unsigned int, std::string> get_attribute_location(const char* name);
        Result<unsigned int, std::string> get_uniform_location(const char* name);

    private:
        Shader(unsigned int program, unsigned int vs, unsigned int fs);

        unsigned int program, vs, fs;
    };
}