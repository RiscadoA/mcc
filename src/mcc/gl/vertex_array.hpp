#pragma once

#include <string>

#include <mcc/result.hpp>
#include <mcc/gl/vertex_buffer.hpp>

namespace mcc::gl {
    class Attribute {
    public:
        enum class Type {
            U8,
            I8,
            NU8, // Normalized U8
            NI8, // Normalized I8

            U32,
            I32,
            NU32, // Normalized U32
            NI32, // Normalized I32
            
            F32,        
        };

        inline Attribute(const VertexBuffer& buffer, size_t stride, size_t offset, int size, Type type, unsigned int shader_location) : 
            buffer(buffer), stride(stride), offset(offset), size(size), type(type), shader_location(shader_location) {
            // Empty
        }

        ~Attribute() = default;

    private:
        friend class VertexArray;

        const VertexBuffer& buffer;
        size_t stride;
        size_t offset;
        

        int size;
        Type type;
        
        unsigned int shader_location;
    };

    class VertexArray final {
    public:
        inline VertexArray() : vao(0) {}
        VertexArray(VertexArray&& rhs);
        VertexArray& operator=(VertexArray&& rhs);
        ~VertexArray();

        static Result<VertexArray, std::string> create(std::initializer_list<Attribute> attributes);
        
        void bind() const;

    private:
        friend class VertexArray;

        VertexArray(unsigned int vao);

        unsigned int vao;
    };
}
