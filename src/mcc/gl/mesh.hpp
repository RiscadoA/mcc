#pragma once

#include <vector>
#include <glm/glm.hpp>

#include <mcc/gl/vertex_array.hpp>
#include <mcc/gl/vertex_buffer.hpp>
#include <mcc/gl/index_buffer.hpp>
#include <mcc/gl/voxel.hpp>

namespace mcc::gl {
    struct Vertex {
        glm::vec3 pos, normal;
        glm::u8vec4 color;
    };

    class Mesh final {
    public:
        Mesh() = default;
        Mesh(const Mesh&) = delete;
        Mesh(Mesh&& rhs);
        ~Mesh() = default;

        void draw_opaque() const;
        void draw_transparent() const;

        void update(const Octree& octree, float root_sz, int lod = -1, bool generate_borders = true);
        void update(const Matrix& matrix, float vx_sz, bool generate_borders = true);
        void update(
            const std::vector<Vertex>& vertices,
            const std::vector<unsigned int>& opaque_indices,
            const std::vector<unsigned int>& transparent_indices
        );

    private:
        gl::VertexArray va;
        gl::VertexBuffer vb;
        gl::IndexBuffer ib; 
        
        int opaque_count, transparent_count, transparent_offset;
    };
}