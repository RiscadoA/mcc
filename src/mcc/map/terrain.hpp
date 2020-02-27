#pragma once

#include <list>
#include <glm/glm.hpp>
#include <mcc/map/chunk.hpp>

namespace mcc::map {
    // Loads and unloads chunks as needed
    class Terrain {
    public:
        Terrain();
        ~Terrain() = default;
        
        void draw(const glm::mat4& vp);
        void update(float dt);
        
    private:
        std::list<Chunk> loaded_chunks;
    };
}