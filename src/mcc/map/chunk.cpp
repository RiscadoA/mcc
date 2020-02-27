#include <mcc/map/chunk.hpp>

#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>

using namespace mcc;
using namespace mcc::map;

Chunk::Chunk(const glm::i64vec3& pos, gl::Shader& shader, const Block::Registry& registry) : 
    pos(pos), first_state(nullptr), first_entity(nullptr), shader(shader), registry(registry) {
    this->ref_count = 0;
    this->lifetime = 0.0f;
    this->vert_count = 0;

    // 36 vertices per cube in chunk maximum
    this->vb = gl::VertexBuffer::create(Chunk::Size * Chunk::Size * Chunk::Size * 6 * 6 * sizeof(float) * 6, nullptr, gl::Usage::Dynamic).unwrap();
    this->va = gl::VertexArray::create({
        gl::Attribute(
            this->vb,
            sizeof(float) * 6, sizeof(float) * 0,
            3, gl::Attribute::Type::F32,
            this->shader.get_attribute_location("position").unwrap()
        ),
        gl::Attribute(
            this->vb,
            sizeof(float) * 6, sizeof(float) * 3,
            3, gl::Attribute::Type::F32,
            this->shader.get_attribute_location("color").unwrap()
        )
    }).unwrap();
}

Chunk::~Chunk() {
    // Kill entities
    // TO DO
}

void Chunk::draw(const glm::mat4& vp) {
    // Get transformation matrix
    glm::mat4 mvp = glm::translate(vp, glm::vec3(this->pos) * glm::vec3(Chunk::Size * Block::Size));
    glUniformMatrix4fv(this->shader.get_uniform_location("mvp").unwrap(), 1, GL_FALSE, &mvp[0][0]);
    
    // Draw chunk
    this->va.bind();
    glDrawArrays(GL_TRIANGLES, 0, GLsizei(this->vert_count));
}

void Chunk::update(float dt) {
    if (this->ref_count <= 0) {
        this->lifetime -= dt;
    }

    // Update block states (entities should be updated by the entity manager)
    
}

void Chunk::load(std::istream& in) {
    // TO DO
}

void Chunk::generate(const Generation& generation) {
    if (this->pos.y < -1) {
        for (int x = 0; x < Chunk::Size; ++x) {
            for (int z = 0; z < Chunk::Size; ++z) {
                for (int y = 0; y < Chunk::Size; ++y) {
                    this->blocks[x][y][z] = this->registry["stone"].unwrap().get_id();
                }
            }
        }
    } else if (this->pos.y == -1) {
        for (int x = 0; x < Chunk::Size; ++x) {
            for (int z = 0; z < Chunk::Size; ++z) {
                for (int y = 0; y < Chunk::Size - 1; ++y) {
                    this->blocks[x][y][z] = this->registry["dirt"].unwrap().get_id();
                }

                this->blocks[x][Chunk::Size - 1][z] = this->registry["grass"].unwrap().get_id();
            }
        }
    } else {
        for (int x = 0; x < Chunk::Size; ++x) {
            for (int z = 0; z < Chunk::Size; ++z) {
                for (int y = 0; y < Chunk::Size; ++y) {
                    this->blocks[x][y][z] = this->registry["air"].unwrap().get_id();
                }
            }
        }
    }

    this->build();
}

void Chunk::unload(std::ostream& out) {
    // Unload blocks
    // TO DO
    // Unload entities
    // TO DO
}

void Chunk::build() {
    struct Vert {
        glm::vec3 pos, color;
    };
    
    auto mem = (Vert*)this->vb.map().unwrap();
    this->vert_count = 0;

    //  If block is not opaque:
    //      For each face:
    //          Draw neighbour block face

    for (int x = 0; x < Chunk::Size; ++x) {
        for (int y = 0; y < Chunk::Size; ++y) {
            for (int z = 0; z < Chunk::Size; ++z) {
                auto& type = this->registry[this->blocks[x][y][z]];
                
                if (!type.is_opaque()) {
                    continue;
                }

                if (x == Chunk::Size - 1 || !this->registry[this->blocks[x + 1][y][z]].is_opaque()) { // Add right face
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) + 0.5f) * Block::Size, (float(y) - 0.5f) * Block::Size, (float(z) - 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) + 0.5f) * Block::Size, (float(y) + 0.5f) * Block::Size, (float(z) + 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) + 0.5f) * Block::Size, (float(y) - 0.5f) * Block::Size, (float(z) + 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) + 0.5f) * Block::Size, (float(y) + 0.5f) * Block::Size, (float(z) + 0.5f) * Block::Size };        
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) + 0.5f) * Block::Size, (float(y) - 0.5f) * Block::Size, (float(z) - 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) + 0.5f) * Block::Size, (float(y) + 0.5f) * Block::Size, (float(z) - 0.5f) * Block::Size };
                }

                if (x == 0 || !this->registry[this->blocks[x - 1][y][z]].is_opaque()) { // Add left face
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) - 0.5f) * Block::Size, (float(y) - 0.5f) * Block::Size, (float(z) - 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) - 0.5f) * Block::Size, (float(y) - 0.5f) * Block::Size, (float(z) + 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) - 0.5f) * Block::Size, (float(y) + 0.5f) * Block::Size, (float(z) + 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) - 0.5f) * Block::Size, (float(y) + 0.5f) * Block::Size, (float(z) + 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) - 0.5f) * Block::Size, (float(y) + 0.5f) * Block::Size, (float(z) - 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) - 0.5f) * Block::Size, (float(y) - 0.5f) * Block::Size, (float(z) - 0.5f) * Block::Size };
                }

                if (y == Chunk::Size - 1 || !this->registry[this->blocks[x][y + 1][z]].is_opaque()) { // Add top face
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) - 0.5f) * Block::Size, (float(y) + 0.5f) * Block::Size, (float(z) - 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) + 0.5f) * Block::Size, (float(y) + 0.5f) * Block::Size, (float(z) + 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) + 0.5f) * Block::Size, (float(y) + 0.5f) * Block::Size, (float(z) - 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) + 0.5f) * Block::Size, (float(y) + 0.5f) * Block::Size, (float(z) + 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) - 0.5f) * Block::Size, (float(y) + 0.5f) * Block::Size, (float(z) - 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) - 0.5f) * Block::Size, (float(y) + 0.5f) * Block::Size, (float(z) + 0.5f) * Block::Size };
                }
                
                if (y == 0 || !this->registry[this->blocks[x][y - 1][z]].is_opaque()) { // Add bottom face
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) - 0.5f) * Block::Size, (float(y) - 0.5f) * Block::Size, (float(z) - 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) + 0.5f) * Block::Size, (float(y) - 0.5f) * Block::Size, (float(z) - 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) + 0.5f) * Block::Size, (float(y) - 0.5f) * Block::Size, (float(z) + 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) + 0.5f) * Block::Size, (float(y) - 0.5f) * Block::Size, (float(z) + 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) - 0.5f) * Block::Size, (float(y) - 0.5f) * Block::Size, (float(z) + 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) - 0.5f) * Block::Size, (float(y) - 0.5f) * Block::Size, (float(z) - 0.5f) * Block::Size };
                }

                if (z == Chunk::Size - 1 || !this->registry[this->blocks[x][y][z + 1]].is_opaque()) { // Add front face
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) - 0.5f) * Block::Size, (float(y) - 0.5f) * Block::Size, (float(z) + 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) + 0.5f) * Block::Size, (float(y) - 0.5f) * Block::Size, (float(z) + 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) + 0.5f) * Block::Size, (float(y) + 0.5f) * Block::Size, (float(z) + 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) + 0.5f) * Block::Size, (float(y) + 0.5f) * Block::Size, (float(z) + 0.5f) * Block::Size };        
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) - 0.5f) * Block::Size, (float(y) + 0.5f) * Block::Size, (float(z) + 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) - 0.5f) * Block::Size, (float(y) - 0.5f) * Block::Size, (float(z) + 0.5f) * Block::Size };
                }

                if (z == 0 || !this->registry[this->blocks[x][y][z - 1]].is_opaque()) { // Add back face
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) - 0.5f) * Block::Size, (float(y) - 0.5f) * Block::Size, (float(z) - 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) + 0.5f) * Block::Size, (float(y) + 0.5f) * Block::Size, (float(z) - 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) + 0.5f) * Block::Size, (float(y) - 0.5f) * Block::Size, (float(z) - 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) + 0.5f) * Block::Size, (float(y) + 0.5f) * Block::Size, (float(z) - 0.5f) * Block::Size };        
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) - 0.5f) * Block::Size, (float(y) - 0.5f) * Block::Size, (float(z) - 0.5f) * Block::Size };
                    mem[this->vert_count].color = type.get_color();
                    mem[this->vert_count++].pos = { (float(x) - 0.5f) * Block::Size, (float(y) + 0.5f) * Block::Size, (float(z) - 0.5f) * Block::Size };
                }
            }
        }
    }
    
    vb.unmap();
}

bool Chunk::is_ready() const {
    return this->ready;
}

void Chunk::add_entity(entity::Entity* entity) {
    if (this->first_entity == nullptr) {
        this->first_entity = entity;
        entity->prev = nullptr;
        entity->next = nullptr;
    } else {
        entity->prev = nullptr;
        entity->next = this->first_entity;
        this->first_entity->prev = entity;
        this->first_entity = entity;
    }
}

void Chunk::remove_entity(entity::Entity* entity) {
    if (entity->next != nullptr) {
        entity->next->prev = entity->prev;
    }

    if (entity == this->first_entity) {
        this->first_entity = entity->next;
    } else {
        entity->prev->next = entity->next;
    }

    entity->prev = nullptr;
    entity->next = nullptr;
}
