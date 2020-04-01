#include <mcc/map/chunk.hpp>
#include <mcc/map/terrain.hpp>

#include <vector>
#include <chrono>

#include <glm/gtc/noise.hpp>

using namespace mcc;
using namespace mcc::map;

Chunk::Chunk(const Generator& generator, glm::i64vec3 position) :
    generator(generator), position(position) {
    this->loaded = false;
    this->has_fence = false;
}

Chunk::~Chunk() {

}

static float simplex(glm::vec3 p, int k) {
    return (glm::simplex(p / float(1 << (2 + k))) + 1.0f) / 2.0f;
}

static float simplex(glm::vec2 p, int k) {
    return (glm::simplex(p / float(1 << (2 + k))) + 1.0f) / 2.0f;
}

void Chunk::load() {
    auto begin = std::chrono::steady_clock::now();

    auto csz = int(this->generator.get_chunk_size());

    auto world_pos = this->position * long long(csz);

    this->voxels.resize(csz * csz * csz, glm::u8vec4(0, 0, 0, 0)); // Initialize empty
    for (auto x = 0ll, wx = world_pos.x; x < csz; ++x, ++wx) {
        for (auto z = 0ll, wz = world_pos.z; z < csz; ++z, ++wz) {
            auto h = simplex({ wx, wz }, 3) * 2 + simplex({ wx, wz }, 5) * 8 + simplex({ wx, wz }, 6) * 32;

            for (auto y = 0ll, wy = world_pos.y; y < csz; ++y, ++wy) {
                const float w = 0.5f;

                if (wy < h &&
                    glm::cos((w/(2.0 * 3.14159f)) * float(wx)) +
                    glm::cos((w/(2.0 * 3.14159f)) * float(wy)) +
                    glm::cos((w/(2.0 * 3.14159f)) * float(wz)) < 0) {
                    auto c = glm::u8vec4(150, 150, 150, 255);
                    if (h - wy <= 1) {
                        c = glm::u8vec4(0, 230, 40, 255);
                    } else if (h - wy <= 3) {
                        c = glm::u8vec4(230, 150, 40, 255);
                    }
                    this->voxels[x * csz * csz + y * csz + z] = c;
                } else {
                    this->voxels[x * csz * csz + y * csz + z] = glm::u8vec4(0, 0, 0, 0);
                }
            }
            /*auto h = simplex({ wx, wz }, 3) * 2 + simplex({ wx, wz }, 5) * 8 + simplex({ wx, wz }, 6) * 32;
            auto c = glm::clamp(simplex({ wx, wz }, 3), 0.0f, 1.0f);

            if (wx > 0 && wx < 50 && wz > 0 && wz < 50) {
                h -= 500;
            }


            auto y = 0ll, wy = world_pos.y;
            for (; wy <= h && y < csz; ++y, ++wy) {
                this->voxels[x * csz * csz + y * csz + z] = glm::u8vec4(
                    glm::mix(
                        glm::vec3(200, 255, 0),
                        glm::vec3(50, 150, 0),
                        glm::clamp(float(wy / 20.0f), 0.0f, 1.0f)
                    )
                    , 255);
            }

            for (; wy <= 4 && y < csz; ++y, ++wy) {
                this->voxels[x * csz * csz + y * csz + z] = glm::u8vec4(0, 200, 255, 100);
            }*/
        }
    }

    /*for (int i = 0, s = csz * csz * csz; i < s; ++i) {
        this->voxels[i] = (i % 2) ? glm::u8vec4(255, 255, 255, 255) : glm::u8vec4(0, 0, 0, 0);
    }/*

    /*for (auto x = 0; x < csz; ++x) {
        for (auto z = 0; z < csz ; ++z) {
            for (auto y = 0; y < csz / 2; ++y) {
                this->voxels[x * csz * csz + y * csz + z] = glm::u8vec4(255, 255, 255, 255);
            }
        }
    }*/

    /*auto end = std::chrono::steady_clock::now();
    auto t1 = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    begin = end;*/

    this->build_buffers(&this->voxels[0], glm::uvec3(csz, csz, csz), Terrain::VoxelSize);
    
    this->sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    this->has_fence = true;

    /*end = std::chrono::steady_clock::now();
    auto t2 = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

    std::cout << "Loaded chunk ( " << this->position.x << " ; " << this->position.y << " ; " << this->position.z <<  " ):\n";
    std::cout << "Generation time: " << t1 << "ms / Meshing time: " << t2 << "ms" << std::endl;*/
}

void Chunk::draw() {
    this->Mesh::draw();
}

bool Chunk::is_loaded() {
    if (this->loaded) {
        return true;
    }

    if (!this->has_fence) {
        return false;
    }

    auto state = glClientWaitSync(this->sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
    if (state == GL_TIMEOUT_EXPIRED) {
        return false;
    } else if (state == GL_WAIT_FAILED) {
        std::cerr << "Chunk::is_loaded() failed:\nglClientWaitSync() returned GL_WAIT_FAILED:\nglGetError() returned " << glGetError() << '\n';
        std::abort();
    } else {
        this->build_va();
        this->loaded = true;
        return true;
    }
}

glm::u8vec4 Chunk::get_voxel(glm::uvec3 pos) const {
    auto csz = this->generator.get_chunk_size();
    return this->voxels[pos.x * csz * csz + pos.y * csz + pos.z];
}