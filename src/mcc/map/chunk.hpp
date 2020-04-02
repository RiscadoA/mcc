#pragma once

#include <glm/glm.hpp>
#include <atomic>
#include <vector>
#include <GL/glew.h>

#include <mcc/map/generator.hpp>
#include <mcc/gl/mesh.hpp>

namespace mcc::map {
    class Chunk {
    public:
        Chunk(const Generator& generator, glm::i64vec3 position); // Initializes an unloaded chunk.
        ~Chunk();

        void load(); // Loads the chunk at a certain level of detail
        void draw();

        inline const glm::i64vec3& get_position() const { return this->position; }
        bool is_loaded();

        glm::u8vec4 get_voxel(glm::uvec3 pos) const;
        
    private:    
        const Generator& generator;
        const glm::i64vec3 position;
    
        std::vector<glm::u8vec4> voxels;
        std::atomic<bool> loaded, has_fence;
        GLsync sync;

        gl::Mesh mesh;
    };
}