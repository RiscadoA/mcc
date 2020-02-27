#pragma once

#include <list>
#include <queue>
#include <future>
#include <thread>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include <mcc/map/chunk.hpp>
#include <mcc/gl/shader.hpp>

namespace mcc::map {
    // Loads and unloads chunks as needed
    class Terrain {
    public:
        Terrain(const Generation& generation, const Block::Registry& registry);
        ~Terrain();
        
        // References a chunk and sets its lifetime to at least the value specified.
        // The chunk won't be immediatelly retrieved (this is an async operation), as it takes time to load/generate.
        // When the chunk isn't needed anymore, dereference should be called.
        Chunk& reference(glm::i64vec3 position, float lifetime = 10.0f);
        void dereference(Chunk& chunk);

        void draw(const glm::mat4& vp);
        void update(float dt);
        
    private:
        static void load_thread_func(Terrain& terrain, GLFWwindow* context);

        const Block::Registry& registry;
        Generation generation;
        
        std::recursive_mutex list_mutex;
        std::list<Chunk> chunks; // Loaded/loading chunks

        std::mutex queue_mutex;
        std::condition_variable queue_cv;
        std::queue<Chunk*> load_queue;
        std::thread load_thread;
        std::atomic<bool> thread_stop;

        gl::Shader shader;
    };
}