#pragma once

#include <vector>
#include <glm/glm.hpp>

#include <mcc/gl/vertex_array.hpp>
#include <mcc/gl/vertex_buffer.hpp>
#include <mcc/gl/index_buffer.hpp>

namespace mcc::gl {
    class Mesh final {
    public:
        Mesh() = default;
        Mesh(const Mesh&) = delete;
        Mesh(Mesh&& rhs);
        ~Mesh() = default;

        void draw() const;

        void build_buffers(const glm::u8vec4* voxels, glm::uvec3 sz, float vx_sz, bool generate_borders = true);
        void build_va();

    private:
        gl::VertexArray va;
        gl::VertexBuffer vb;
        gl::IndexBuffer ib;
        int index_count;
    };
}