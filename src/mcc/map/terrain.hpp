#pragma once

#include <mcc/ui/camera.hpp>
#include <mcc/gl/vertex_array.hpp>
#include <mcc/gl/shader.hpp>
#include <mcc/map/chunk.hpp>
#include <mcc/map/generator.hpp>

#include <thread>
#include <mutex>
#include <map>
#include <queue>

namespace std {
    template <>
    struct less<glm::i64vec3> {
        inline bool operator()(const glm::i64vec3& lhs, const glm::i64vec3& rhs) const {
            return lhs.x < rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y) || (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z < rhs.z);
        }
    };
}

namespace mcc::map {
    class Terrain {
    public:
        static constexpr float VoxelSize = 1.0f;

        Terrain(const Generator& generator, const gl::Shader& shader);
        ~Terrain();
        
        // Loads new chunks and unloads old ones
        void update(const ui::Camera& camera);

        // Draws the currently loaded chunks
        void draw(const ui::Camera& camera);

        glm::u8vec4 get_voxel(glm::i64vec3 pos) const;

    private:
        static void load_thread_func(Terrain& terrain, void* context);

        // Main chunks
        //std::queue<Chunk*> va_queue;
        //std::priority_queue<std::pair<float, Chunk*>> vb_queue; // 

        const Generator& generator;
        const gl::Shader& shader;
        
        std::map<glm::i64vec3, Chunk*> map;

        std::queue<Chunk*> queue;
        std::mutex queue_mutex;
        std::condition_variable queue_cv;
        std::thread load_thread;
        std::atomic<bool> thread_stop;
    };
}
