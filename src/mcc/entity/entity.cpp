#include <mcc/entity/entity.hpp>

using namespace mcc;
using namespace mcc::entity;

Entity::Entity(glm::i64vec3 chunk_pos, glm::f32vec3 pos) :
    chunk_pos(chunk_pos), pos(pos) {
    
}
