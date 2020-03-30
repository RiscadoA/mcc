#pragma once

#include <vector>
#include <glm/glm.hpp>

#include <mcc/gl/vertex_array.hpp>
#include <mcc/gl/vertex_buffer.hpp>
#include <mcc/gl/index_buffer.hpp>

namespace mcc::gl {
    class Mesh {
    public:
        Mesh() = default;
        virtual ~Mesh() = default;

        void draw();

    protected:
        void build_mesh(const glm::u8vec4* voxels, glm::uvec3 sz, float vx_sz, bool generate_borders = true);
        
    private:
        gl::VertexArray va;
        gl::VertexBuffer vb;
        gl::IndexBuffer ib;
        int index_count;
    };
}