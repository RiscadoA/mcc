#pragma once

#include <mcc/gl/mesh.hpp>
#include <mcc/ui/camera.hpp>
#include <mcc/map/generator.hpp>

#include <glm/glm.hpp>
#include <GL/glew.h>

namespace mcc::map {
    class Chunk final {
    public:
        Chunk(Generator& generator, Chunk* parent, glm::f64vec3 center, float vox_sz, int chunk_size, int level);
        ~Chunk();

        void generate();
        void update(const ui::Camera& camera, float lod_distance);
        void draw(const ui::Camera& camera, unsigned int model_loc);

        inline Chunk* get_parent() const { return this->parent; }

        inline float get_score() const { return this->score; }
        inline int get_level() const { return this->level; }
        inline bool should_delete() const { return this->delete_flag; }
        inline bool is_generated() const { return this->generated; }

    private:
        void collapse();

        Generator& generator;

        gl::Mesh mesh;
        gl::Matrix matrix;

        Chunk* parent;
        Chunk* children[8];

        glm::f64vec3 center;
        float vox_sz;
        int chunk_size, level;

        bool visible;
        bool generated;
        bool delete_flag;
        float score;

        GLsync sync;
        bool has_fence;
    };
}