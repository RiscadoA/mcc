#include <mcc/map/block.hpp>

using namespace mcc;
using namespace mcc::map;

Block::Block(Chunk* chunk, glm::u8vec3 pos) :
    chunk(chunk), pos(pos), prev(nullptr), next(nullptr) {
    // Empty
}

void Block::update(float dt) {
    // Empty
}

void Block::destroy() {
    // Empty
}
