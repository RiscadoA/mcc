#pragma once

#include <atomic>
#include <future>
#include <fstream>
#include <glm/glm.hpp>

#include <mcc/map/block.hpp>
#include <mcc/map/generation.hpp>
#include <mcc/entity/entity.hpp>
#include <mcc/gl/shader.hpp>
#include <mcc/gl/vertex_array.hpp>

namespace mcc::map {
    class Chunk final {
    public:
        static constexpr int Size = 32;
        
        Chunk(const glm::i64vec3& pos, gl::Shader& shader, const Block::Registry& registry);
        ~Chunk(); // If unload() is not called before the destructor, the data in the chunk is discarded
        
        void draw(const glm::mat4& vp);
        void update(float dt); // Updates every block state
        
        void generate(const Generation& generation); // Generates a new chunk at a position 
        void load(std::istream& in); // Loads a chunk from a stream    
        void unload(std::ostream& out); // Unloads chunk data, saving it to the disk
        
        bool is_ready() const; // Checks if the chunk is already loaded
        
        inline int64_t get_reference_count() const { return this->ref_count; }
        inline float get_lifetime() const { return this->lifetime; }
        inline void set_lifetime(float lifetime) { this->lifetime = lifetime; }

        inline glm::i64vec3 get_pos() const { return this->pos; }

        void add_entity(entity::Entity* entity);
        void remove_entity(entity::Entity* entity);

    private:
        friend class Terrain;
        friend class ChunkRef;

        void build();

        glm::i64vec3 pos; // World chunk coordinates
        const Block::Registry& registry;

        std::atomic<bool> ready;
        mutable std::atomic<int64_t> ref_count; // Number of references that prevent a chunk from being unloaded
        float lifetime; // Remaining lifetime in seconds (decreased every update if ref_count == 0)

        unsigned short blocks[Chunk::Size][Chunk::Size][Chunk::Size];
        Block* first_state; // Linked list, allocated on heap (should be moved to a pool allocator later)
        entity::Entity* first_entity; // Linked list pointing to entities present on the chunk

        size_t vert_count;
        gl::Shader& shader;
        gl::VertexBuffer vb;
        gl::VertexArray va;
    };
}