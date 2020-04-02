#include <mcc/map/terrain.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <set>

/*
    Loader thread:
    - loop:
        - lock queue
        - pop chunk
        - unlock queue
        - load chunk
    Main thread:
    - loop:
        - add new chunks to queue
        - lock queue
        - calculate priority for loaded chunks and queued chunks
        - sort chunk queue
        - unload/remove from queue all of the chunks with negative priority
        - unlock queue
        -------------------------------------
        - 
    
    
*/

using namespace mcc;
using namespace mcc::map;

void Terrain::load_thread_func(Terrain& terrain, void* context) {
    glfwMakeContextCurrent((GLFWwindow*)(context));

    std::unique_lock unique_lock(terrain.queue_mutex);

    for (;;) {
        while (!terrain.queue.empty()) {
            auto chunk = terrain.queue.front();
            terrain.queue.pop();
            unique_lock.unlock();
            chunk->load();
            unique_lock.lock();
        }

        terrain.queue_cv.wait(unique_lock);
        if (terrain.thread_stop) {
            break;
        }
    }
}

Terrain::Terrain(const Generator& generator, const gl::Shader& shader) :
    generator(generator), shader(shader) {

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

void Terrain::update(const ui::Camera& camera) {
    auto render_distance = long long(std::ceil(camera.get_z_far() / (this->generator.get_chunk_size() * Terrain::VoxelSize))) + 1;
    auto position = glm::i64vec3(camera.get_position() / (this->generator.get_chunk_size() * Terrain::VoxelSize));

    // Collect every candidate chunk to be loaded
    std::vector<std::pair<float, glm::i64vec3>> candidates;
    candidates.reserve(render_distance * render_distance * render_distance);

    for (long long x = position.x - render_distance; x <= position.x + render_distance; ++x) {
        for (long long y = position.y - render_distance; y <= position.y + render_distance; ++y) {
            for (long long z = position.z - render_distance; z <= position.z + render_distance; ++z) {
                auto coords = glm::i64vec3(x, y, z);
                if (this->map.find(coords) == this->map.end()) {
                    float distance = glm::distance(glm::vec3(position), glm::vec3(coords));
                    if (distance > render_distance) {
                        continue;
                    }

                    // If the chunk is outside of the camera frustum, leave it with a low priority
                    auto world_pos = (glm::vec3(coords) + 0.5f) * float(this->generator.get_chunk_size()) * Terrain::VoxelSize;
                    if (!camera.intersects_frustum(world_pos, this->generator.get_chunk_size() * Terrain::VoxelSize)) {
                        distance += render_distance;
                    }

                    candidates.push_back({ distance, coords });
                }
            }
        }
    }

    
    if (!candidates.empty()) {
        std::sort(candidates.begin(), candidates.end(), [](auto& lhs, auto& rhs) -> bool { return lhs.first < rhs.first; });

        this->queue_mutex.lock();
        for (auto& c : candidates) {
            auto chunk = new Chunk(this->generator, c.second);
            this->map.emplace(std::make_pair(c.second, chunk));
            this->queue.push(chunk);
        }
        this->queue_mutex.unlock();
        this->queue_cv.notify_all();
    }

    // Clean old chunks
    for (auto it = this->map.begin(); it != this->map.end();) {
        auto c = it++;
        if (!c->second->is_loaded()) {
            continue;
        }

        auto distance = glm::distance(glm::vec3(c->second->get_position()), glm::vec3(position));
        if (distance > float(render_distance)) {
            delete c->second;
            this->map.erase(c);
        }     
    }
}

void Terrain::draw(const ui::Camera& camera) {
    this->shader.bind();

    auto vp_index = this->shader.get_uniform_location("vp").unwrap();
    auto vp = camera.get_projection() * camera.get_view();
    glUniformMatrix4fv(vp_index, 1, GL_FALSE, &vp[0][0]);

    auto model_index = this->shader.get_uniform_location("model").unwrap();
    glm::mat4 model;

    for (auto& c : this->map) {
        if (c.second->is_loaded()) {
            // If the chunk is outside of the camera frustum, dont draw it
            auto world_pos = (glm::vec3(c.first) + 0.5f) * float(this->generator.get_chunk_size()) * Terrain::VoxelSize;
            if (!camera.intersects_frustum(world_pos, this->generator.get_chunk_size() * Terrain::VoxelSize)) {
                continue;
            }

            model = glm::translate(glm::mat4(1.0f), glm::vec3(c.second->get_position()) * float(this->generator.get_chunk_size()));
            glUniformMatrix4fv(model_index, 1, GL_FALSE, &model[0][0]);
            c.second->draw();
        }
    }
}

glm::u8vec4 Terrain::get_voxel(glm::i64vec3 pos) const {
    auto csz = long long(this->generator.get_chunk_size());
    auto chunk_coords = pos / csz;
    auto voxel_coords = pos % csz;

    if (voxel_coords.x < 0) {
        voxel_coords.x += csz;
    } else if (voxel_coords.y < 0) {
        voxel_coords.y += csz;
    } else if (voxel_coords.z < 0) {
        voxel_coords.z += csz;
    }

    for (auto& c : this->map) {
        if (!c.second->is_loaded()) {
            continue;
        }

        if (c.second->get_position() == chunk_coords) {
            return c.second->get_voxel(glm::uvec3(voxel_coords));
        }
    }

    return glm::u8vec4(0, 0, 0, 0);
}
