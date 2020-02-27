#pragma once

#include <glm/glm.hpp>
#include <fstream>
#include <vector>
#include <string>

#include <mcc/result.hpp>

namespace mcc::map {
    class Chunk;

    // Base class for block states
    class Block {
    public:
        static constexpr float Size = 1;

        class Type {
        public:
            Type(unsigned short id, const std::string& name, const glm::vec3& color, bool opaque);

            inline unsigned short get_id() const { return this->id; }
            inline std::string get_name() const { return this->name; }
            inline const glm::vec3& get_color() const { return this->color; }
            inline bool is_opaque() const { return this->opaque; }

        private:
            unsigned short id;
            std::string name;
            glm::vec3 color;
            bool opaque;
        };

        class Registry {
        public:
            Registry();
            ~Registry() = default;
            
            void load(std::istream& in); // TO DO

            Result<const Block::Type&, std::string> operator[](const char* name) const;
            const Block::Type& operator[](unsigned short id) const;

        private:
            std::vector<Block::Type> block_types;
        };

        Block(Chunk* chunk, glm::u8vec3 pos);
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
        inline glm::u8vec3 get_pos() const { return this->pos; }
        
        inline Block* get_prev() { return this->prev; }
        inline const Block* get_prev() const { return this->prev;}
        inline Block* get_next() { return this->prev; }
        inline const Block* get_next() const { return this->prev;}

    private:
        friend Chunk;

        Block* prev;
        Block* next;
        Chunk* chunk;

        glm::u8vec3 pos;
    };
}