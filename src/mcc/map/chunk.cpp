#include <mcc/map/chunk.hpp>

#include <mcc/gl/debug.hpp>

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

mcc::map::Chunk::Chunk(Generator& generator, Chunk* parent, glm::f64vec3 center, float vox_sz, int chunk_size, int level)
    : generator(generator), parent(parent), center(center), vox_sz(vox_sz), chunk_size(chunk_size), level(level) {
    this->score = +INFINITY;
    for (int i = 0; i < 8; ++i) {
        this->children[i] = nullptr;
    }
    this->generated = false;
    this->has_fence = false;
    this->visible = false;
    this->generator.load(this);
}

mcc::map::Chunk::~Chunk() {
    this->generator.unload(this);
}

void mcc::map::Chunk::generate() {
    this->generator.generate_palette(this->center, this->level, this->matrix.palette);
    this->matrix.size = glm::u8vec3(this->chunk_size, this->chunk_size, this->chunk_size);
    this->matrix.voxels.resize(this->chunk_size * this->chunk_size * this->chunk_size);

    for (int x = 0, i = 0; x < this->chunk_size; ++x) {
        for (int y = 0; y < this->chunk_size; ++y) {
            for (int z = 0; z < this->chunk_size; ++z, ++i) {
                auto offset = (glm::f64vec3(x, y, z) / (double)this->chunk_size) - glm::f64vec3(0.5);
                offset *= this->chunk_size * this->vox_sz;
                this->matrix.voxels[i] = this->generator.generate_material(offset + this->center, this->level);
            }
        }
    }

    this->mesh.update(this->matrix, this->vox_sz, true, false);

    this->sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    this->has_fence = true;
}

void mcc::map::Chunk::update(const ui::Camera& camera, float lod_distance) {
    auto offset = camera.get_position() - glm::vec3(this->center);
    auto distance = glm::length(offset);

    if (!this->generated) {
        this->visible = false;
        this->score = distance * distance - this->level * 100;
        
        if (this->has_fence) {
            auto state = glClientWaitSync(this->sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
            if (state == GL_WAIT_FAILED) {
                std::cerr << "mcc::map::Chunk::update() failed:" << std::endl;
                std::cerr << "glClientWaitSync() returned GL_WAIT_FAILED:" << std::endl;
                std::cerr << "glGetError() returned " << glGetError() << std::endl;
                std::abort();
            }
            else if (state != GL_TIMEOUT_EXPIRED) {
                this->generated = true;
                this->mesh.generate_va();
            }
            else {
                return;
            }
        }
        else {
            return;
        }
    }

    // Check if this chunk should be further divided
    bool divide = false;
    if (this->level > 0) {
        float divide_distance = this->vox_sz * this->chunk_size * lod_distance;
        divide = distance < divide_distance;
    }

    if (this->children[0] == nullptr && divide) {
        // Divide chunk
        for (int i = 0; i < 8; ++i) {
            int x = (i / 4) * 2 - 1;
            int y = ((i % 4) / 2) * 2 - 1;
            int z = (i % 2) * 2 - 1;
            this->children[i] = new Chunk(
                generator,
                this,
                this->center + glm::f64vec3(x, y, z) * (double)this->vox_sz * (double)this->chunk_size * 0.25,
                this->vox_sz / 2.0f,
                this->chunk_size,
                this->level - 1
            );
        }
    }
    else if (this->children[0] != nullptr && !divide) {
        // Collapse chunk
        for (int i = 0; i < 8; ++i) {
            delete this->children[i];
            this->children[i] = nullptr;
        }
    }

    // Calculate visibility
    this->visible = (this->parent == nullptr || this->parent->visible)
        && camera.intersects_frustum(this->center, this->vox_sz * this->chunk_size);

    // Update children
    if (divide) {
        this->score = +INFINITY;
        for (int i = 0; i < 8; ++i) {
            this->children[i]->update(camera, lod_distance);
            this->score = std::min(this->score, this->children[i]->score);
        }
    }
}

void mcc::map::Chunk::draw(const ui::Camera& camera, unsigned int model_loc) {
    if (!this->visible) {
        return;
    }

    bool draw_children = this->children[0] != nullptr;
    if (draw_children) {
        for (int i = 0; i < 8; ++i) {
            if (!this->children[i]->generated) {
                draw_children = false;
                break;
            }
        }
    }

    if (draw_children) {
        for (int i = 0; i < 8; ++i) {
            this->children[i]->draw(camera, model_loc);
        }
    }
    else {
        auto model = glm::translate(
            glm::mat4(1.0f),
            glm::vec3(this->center) - glm::vec3(this->vox_sz * this->chunk_size) * 0.5f
        );
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, &model[0][0]);
        this->mesh.draw_opaque();

        gl::Debug::draw_box(this->center, glm::vec3(this->vox_sz * this->chunk_size) * 0.5f, glm::vec4(0.0f, 1.0f, 0.0f, 0.2f));
    }
}
