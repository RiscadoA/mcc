#pragma once

#include <cstdint>
#include <fstream>
#include <glm/glm.hpp>

namespace mcc::entity {
    class Entity {
    public:
        Entity(glm::i64vec3 chunk_pos, glm::f32vec3 pos);
        virtual ~Entity() = default;

        virtual void load(std::istream& in) = 0;
        virtual void unload(std::ostream& out) = 0;

        // Returns the coordinates of the chunk the entity is present in
        inline glm::i64vec3 get_chunk_position() const { return this->chunk_pos; }
        // Returns the coordinates of the entity inside the chunk
        inline glm::f32vec3 get_position() const { return this->pos; }

        inline Entity* get_prev() { return this->prev; }
        inline const Entity* get_prev() const { return this->prev;}
        inline Entity* get_next() { return this->prev; }
        inline const Entity* get_next() const { return this->prev;}

    private:
        glm::i64vec3 chunk_pos;
        glm::f32vec3 pos;

        Entity* prev;
        Entity* next;
    };
}