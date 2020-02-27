#include <mcc/map/chunk.hpp>

using namespace mcc;
using namespace mcc::map;

Chunk::Chunk(std::istream& in, glm::i64vec3 pos) : 
    pos(pos), first_state(nullptr), first_entity(nullptr) {
    
}

Chunk::Chunk(const Generation& generation, glm::i64vec3 pos) :
    pos(pos), first_state(nullptr), first_entity(nullptr) {
    
}

Chunk::~Chunk() {

}

void Chunk::update(float dt) {

}

void Chunk::unload(std::ostream& out) {
    // Unload blocks
    // TO DO
    // Unload entities
    // TO DO
}
