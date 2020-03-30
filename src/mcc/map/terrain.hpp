#pragma once

#include <mcc/ui/camera.hpp>
#include <mcc/gl/vertex_array.hpp>
#include <mcc/gl/shader.hpp>
#include <mcc/map/chunk.hpp>
#include <mcc/map/generator.hpp>

#include <map>

namespace mcc::map {
    class Terrain {
    public:
        Terrain(const Generator& generator);
        ~Terrain();
        
        // Loads new chunks and unloads old ones
        void update(const ui::Camera& camera);

        // Draws the currently loaded chunks
        void draw(const ui::Camera& camera);

    private:
        const Generator& generator;

        gl::Shader shader;
        Chunk chunk;
    };
}
