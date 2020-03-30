#pragma once

#include <string>

#include <mcc/result.hpp>
#include <mcc/gl/usage.hpp>

namespace mcc::gl {
    class IndexBuffer final {
    public:
        inline IndexBuffer() : ibo(0) {}
        IndexBuffer(IndexBuffer&& rhs);
        IndexBuffer& operator=(IndexBuffer&& rhs);
        ~IndexBuffer();

        static Result<IndexBuffer, std::string> create(size_t size, const void* data, Usage usage);

        Result<void, std::string> update(size_t offset, size_t size, const void* data);
        Result<void*, std::string> map();
        void unmap();
        void bind();

    private:
        IndexBuffer(unsigned int ibo);

        unsigned int ibo;
    };
}
