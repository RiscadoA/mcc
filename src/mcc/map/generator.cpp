#include <mcc/map/generator.hpp>
#include <mcc/map/chunk.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace mcc;
using namespace mcc::map;

mcc::map::Generator::Generator() {
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    auto context = glfwCreateWindow(640, 480, "", nullptr, glfwGetCurrentContext());
    if (context == nullptr) {
        std::cerr << "mcc::map::Generator::Generator() failed:" << std::endl;
        std::cerr << "Couldn't create chunk generator OpenGL context:" << std::endl;
        std::cerr << "glfwCreateWindow() returned nullptr" << std::endl;
        std::abort();
    }
   
    this->current = nullptr;
    this->stop = false;
    this->trigger = false;
    this->thread = std::thread(&Generator::thread_func, this, context);
}

mcc::map::Generator::~Generator() {
    this->stop = true;
    this->thread.join();
}

void mcc::map::Generator::load(Chunk* chunk) {
    chunk_count += 1;
    this->queue_mutex.lock();
    this->trigger = true;
    this->queue.insert(chunk);
    this->queue_mutex.unlock();
}

void mcc::map::Generator::unload(Chunk* chunk) {
    chunk_count -= 1;
    this->queue_mutex.lock();
    auto it = this->queue.find(chunk);
    if (it != this->queue.end()) {
        this->queue.erase(it);
        if (chunk->should_delete()) {
            delete chunk;
        }
    }
    this->queue_mutex.unlock();
}

void mcc::map::Generator::thread_func(void* context) {
    glfwMakeContextCurrent((GLFWwindow*)context);

    while (!this->stop) {
        while (!this->trigger && !this->stop);
        this->queue_mutex.lock();

        if (this->queue.empty()) {
            this->queue_mutex.unlock();
            continue;
        }

        this->current = *this->queue.begin();
        for (auto& c : this->queue) {
            if (c->get_parent() == nullptr) {
                this->current = c;
                break;
            }

            if (c->get_parent()->get_score() < this->current->get_parent()->get_score()) {
                this->current = c;
            }
        }
        this->queue.erase(this->current);
        this->queue_mutex.unlock();

        this->current->generate();
        if (this->current->should_delete()) {
            delete this->current;
        }
        this->current = nullptr;
    }

    glfwDestroyWindow((GLFWwindow*)context);
}