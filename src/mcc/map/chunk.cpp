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

void Chunk::add_entity(entity::Entity* entity) {
    if (this->first_entity == nullptr) {
        this->first_entity = entity;
        entity->prev = nullptr;
        entity->next = nullptr;
    } else {
        entity->prev = nullptr;
        entity->next = this->first_entity;
        this->first_entity->prev = entity;
        this->first_entity = entity;
    }
}

void Chunk::remove_entity(entity::Entity* entity) {
    if (entity->next != nullptr) {
        entity->next->prev = entity->prev;
    }

    if (entity == this->first_entity) {
        this->first_entity = entity->next;
    } else {
        entity->prev->next = entity->next;
    }

    entity->prev = nullptr;
    entity->next = nullptr;
}
