#pragma once

#include <string>

#include <mcc/result.hpp>
#include <mcc/gl/usage.hpp>

namespace mcc::gl {
    class VertexBuffer final {
    public:
        VertexBuffer(VertexBuffer&& rhs);
        ~VertexBuffer();

        static Result<VertexBuffer, std::string> create(size_t size, const void* data, Usage usage);

        Result<void, std::string> update(size_t size, const void* data);
        Result<void*, std::string> map();
        void unmap();

    private:
        friend class VertexArray;

        VertexBuffer(unsigned int vbo);

        unsigned int vbo;
    };
}
