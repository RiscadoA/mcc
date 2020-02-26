#include <mcc/map/block.hpp>

using namespace mcc;
using namespace mcc::map;

Block::Block(Chunk* chunk, uint8_t x, uint8_t y, uint8_t z) :
    chunk(chunk), x(x), y(y), z(z), prev(nullptr), next(nullptr) {
    // Empty
}

void Block::update(float dt) {
    // Empty
}

void Block::destroy() {
    // Empty
}
