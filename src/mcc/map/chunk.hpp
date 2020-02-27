#pragma once

#include <mcc/map/block.hpp>
#include <mcc/map/generation.hpp>
#include <mcc/entity/entity.hpp>

#include <atomic>
#include <fstream>
#include <glm/glm.hpp>

namespace mcc::map {
    class Chunk final {
    public:
        static constexpr int Size = 32;
        
        // Loads a chunk from a stream
        Chunk(std::istream& in, glm::i64vec3 pos);
        // Generates a new chunk at a position
        Chunk(const Generation& generation, glm::i64vec3 pos);
        // If unload() is not called before the destructor, the data in the chunk is discarded
        ~Chunk(); 
        
        void update(float dt); // Updates every block state
        void unload(std::ostream& out); // TO DO: Unloads chunk data, saving it to the disk
        
        inline void reference() const { ++this->ref_count; }
        inline void dereference() const { --this->ref_count; }
        inline int64_t get_reference_count() const { return this->ref_count; }

        inline glm::i64vec3 get_pos() const { return this->pos; }

    private:
        glm::i64vec3 pos; // World chunk coordinates
        mutable std::atomic<int64_t> ref_count; // Number of references that prevent a chunk from being unloaded

        Block::Type blocks[Chunk::Size][Chunk::Size][Chunk::Size];
        Block* first_state; // Linked list, allocated on heap (should be moved to a pool allocator later)
        entity::Entity* first_entity; // Linked list pointing to entities present on the chunk
    };
}