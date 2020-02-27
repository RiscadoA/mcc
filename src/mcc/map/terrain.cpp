#include <GL/glew.h>

#include <mcc/map/terrain.hpp>

using namespace mcc;
using namespace mcc::map;

void Terrain::load_thread_func(Terrain& terrain, GLFWwindow* context) {
    // Init GL context
    glfwMakeContextCurrent(context);

    std::unique_lock unique_lock(terrain.queue_mutex);

    for (;;) {
         // Load all chunks in queue
        while (!terrain.load_queue.empty()) {
            auto chunk = terrain.load_queue.front();
            terrain.load_queue.pop();
            unique_lock.unlock();
            
            // TO DO: load already generated chunks from files
            chunk->generate(terrain.generation);
            chunk->ready = true;
            unique_lock.lock();
        }

        terrain.queue_cv.wait(unique_lock);
        if (terrain.thread_stop) {
            break;
        }
    }
}

Terrain::Terrain(const Generation& generation, const Block::Registry& registry) :
    generation(generation), registry(registry) {
    shader = gl::Shader::create(
        R"(
            #version 330 core

            uniform mat4 mvp;

            layout (location = 0) in vec3 position;
            layout (location = 1) in vec3 color;

            out vec4 gl_Position;
            out vec3 frag_color;

            void main() {
                gl_Position = mvp * vec4(position, 1.0);
                frag_color = color;
            }
        )",
        R"(
            #version 330 core

            in vec3 frag_color;

            out vec4 color;

            void main() {
                color = vec4(frag_color, 1.0);
            }
        )"
    ).unwrap();

    // Launch chunk loader thread
    this->thread_stop = false;
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    auto context = glfwCreateWindow(640, 480, "", nullptr, glfwGetCurrentContext());
    if (context == nullptr) {
        std::cerr << "Terrain::Terrain() failed:\nCouldn't create chunk loader OpenGL context:\nglfwCreateWindow() returned nullptr\n";
        std::abort();
    }
    
    this->load_thread = std::thread(Terrain::load_thread_func, std::ref(*this), context);
}

Terrain::~Terrain() {
    this->thread_stop = true;
    this->queue_cv.notify_all();
    this->load_thread.join();
}

Chunk& Terrain::reference(glm::i64vec3 pos, float lifetime) {
    std::lock_guard lock_guard(this->list_mutex);
    
    // Check if chunk is already loaded/being loaded
    for (auto& chunk : this->chunks) {
        if (chunk.get_pos() == pos) {
            chunk.set_lifetime(glm::max(chunk.get_lifetime(), lifetime));
            chunk.ref_count += 1;
            return chunk;
        }
    }

    // Add chunk to the chunk list and start loading it
    this->chunks.emplace_back(pos, this->shader, this->registry);
    auto& chunk = this->chunks.back();
    chunk.set_lifetime(lifetime);
    chunk.ref_count += 1;
    
    // Load it
    this->queue_mutex.lock();
    this->load_queue.push(&chunk);
    chunk.ready = false;
    this->queue_mutex.unlock();
    this->queue_cv.notify_all();
    return chunk;
}

void Terrain::dereference(Chunk& chunk) {
    chunk.ref_count -= 1;
}

void Terrain::draw(const glm::mat4& vp) {
    std::lock_guard lock_guard(this->list_mutex);

    this->shader.bind();
    
    for (auto& c : chunks) {
        if (c.is_ready()) {
            c.draw(vp);
        }
    }
}

void Terrain::update(float dt) {
    std::lock_guard lock_guard(this->list_mutex);
    for (auto it = chunks.begin(); it != chunks.end();) {
        auto c = it++;
        if (c->is_ready()) {
            c->update(dt);
            if (c->lifetime < 0 && c->ref_count <= 0) {
                // Unload - TO DO
                chunks.erase(c);
            }
        }
    }
}
