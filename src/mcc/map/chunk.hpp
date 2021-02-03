#pragma once

#include <mcc/gl/mesh.hpp>
#include <mcc/ui/camera.hpp>
#include <mcc/map/generator.hpp>

#include <glm/glm.hpp>

namespace mcc::map {
    class Chunk {
    public:
        Chunk(const Generator& generator, int chunk_size, glm::f64vec3 center, int level);
        ~Chunk();

        void update(glm::vec3 view_point, float lod_distance);

        void draw(const ui::Camera& camera);

    private:
        const Generator& generator;

        gl::Mesh mesh;
        gl::Matrix matrix;

        Chunk* children[8];

        glm::f64vec3 center;
        int chunk_size, level;
    };
}