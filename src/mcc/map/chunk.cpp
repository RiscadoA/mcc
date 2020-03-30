#include <mcc/map/chunk.hpp>

#include <vector>
#include <GL/glew.h>

#include <glm/gtc/noise.hpp>

using namespace mcc;
using namespace mcc::map;

static float gen(glm::vec3 pos) {
    return glm::perlin(pos);
}

Chunk::Chunk(const Generator& generator, glm::i64vec3 position) :
    generator(generator), position(position) {
    this->loaded = false;
}

Chunk::~Chunk() {

}

void Chunk::load() {
    auto csz = int(this->generator.get_chunk_size());

    // TO DO: generate voxel grid using some kind of noise
    this->voxels.resize(csz * csz * csz, glm::u8vec4(0, 0, 0, 0)); // Initialize empty
    for (auto x = 0; x < csz; ++x) {
        for (auto y = 0; y < x; ++y) {
            for (auto z = 0; z < csz - 1; ++z) {
                this->voxels[x * csz * csz + y * csz + z] = glm::u8vec4(230, 230, 230, 255);
            }
        }
    }

    this->build_mesh(&this->voxels[0], glm::uvec3(csz, csz, csz), 1.0f);
    this->loaded = true;
}
