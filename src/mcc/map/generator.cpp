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
    this->thread = std::thread(&Generator::thread_func, this, context);
}

mcc::map::Generator::~Generator() {
    this->stop = true;
    this->thread.join();
}

void mcc::map::Generator::load(Chunk* chunk) {
    chunk_count += 1;
    this->queue_mutex.lock();
    this->queue.push_back(chunk);
    this->queue_mutex.unlock();
    std::cout << chunk_count << std::endl;
}

void mcc::map::Generator::unload(Chunk* chunk) {
    chunk_count -= 1;
    this->queue_mutex.lock();
    this->queue.remove(chunk);
    this->queue_mutex.unlock();
    while (this->current == chunk)
        std::cout << "waiting " << chunk << std::endl; // Wait for the chunk to finish loading
    std::cout << chunk_count << std::endl;
}

void mcc::map::Generator::thread_func(void* context) {
    glfwMakeContextCurrent((GLFWwindow*)context);

    while (!this->stop) {
        this->queue_mutex.lock();

        if (this->queue.empty()) {
            this->queue_mutex.unlock();
            continue;
        }

        this->current = this->queue.front();
        for (auto& c : this->queue) {
            if (c->get_score() < this->current->get_score()) {
                this->current = c;
            }
        }
        this->queue.remove(this->current);
        this->queue_mutex.unlock();

        this->current->generate();
        this->current = nullptr;
    }

    glfwDestroyWindow((GLFWwindow*)context);
}