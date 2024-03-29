#pragma once

#include <set>
#include <thread>
#include <mutex>
#include <atomic>

#include <glm/glm.hpp>

#include <mcc/gl/voxel.hpp>

namespace mcc::map {
    class Chunk;

    class Generator {
    public:
        Generator();
        Generator(const Generator&) = delete;
        Generator(Generator&& rhs) = delete;
        ~Generator();

        void load(Chunk* chunk);
        void unload(Chunk* chunk);

        inline Chunk* get_current() const { return this->current; }

        // Receives the chunk center coordinates and its level and generates the palette used.
        virtual void generate_palette(glm::f64vec3 pos, int level, gl::Material* palette) = 0;
        // Receives the voxel's coordinates and its level and generates its material
        virtual unsigned char generate_material(glm::f64vec3 pos, int level) = 0;

    private:
        void thread_func(void* context);
        
        std::thread thread;

        std::atomic<bool> stop;
        std::set<Chunk*> queue;
        Chunk* current;
        std::recursive_mutex queue_mutex;
        bool trigger;

        int chunk_count;
    };
}