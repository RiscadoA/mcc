#pragma once

#include <glm/vec3.hpp>
#include <fstream>

namespace mcc::map {
    class Chunk;

    // Base class for block states
    class Block {
    public:
        enum class Type {
            Empty,
            Stone,
        };

        Block(Chunk* chunk, uint8_t x, uint8_t y, uint8_t z);
        virtual ~Block() = default;

        // Called each game update for each loaded block state
        virtual void update(float dt);
        // Called when the block is destroyed
        virtual void destroy();
        // TO DO - Called on chunk load (loads the block state from a stream)
        virtual void load(std::istream& in) = 0;
        // TO DO - Called on chunk unload (unloads the block state into a stream)
        virtual void unload(std::ostream& out) = 0;

        inline Chunk* get_chunk() { return this->chunk; }
        inline const Chunk* get_chunk() const { return this->chunk; }
        inline uint8_t get_x() const { return this->x; }
        inline uint8_t get_y() const { return this->y; }
        inline uint8_t get_z() const { return this->z; }

        inline Block* get_prev() { return this->prev; }
        inline const Block* get_prev() const { return this->prev;}
        inline Block* get_next() { return this->prev; }
        inline const Block* get_next() const { return this->prev;}

    private:
        friend Chunk;

        Block* prev;
        Block* next;
        Chunk* chunk;
        uint8_t x, y, z;
    };
}