#include <mcc/map/chunk.hpp>

mcc::map::Chunk::Chunk(const Generator& generator, int chunk_size, glm::f64vec3 center, int level)
    : generator(generator), chunk_size(chunk_size), center(center), level(level) {

}

void mcc::map::Chunk::update(glm::vec3 view_point, float lod_distance) {
}

void mcc::map::Chunk::draw(const ui::Camera& camera) {

}
