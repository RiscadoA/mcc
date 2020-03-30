#include <mcc/map/terrain.hpp>
#include <GL/glew.h>

using namespace mcc;
using namespace mcc::map;

Terrain::Terrain(const Generator& generator) :
    generator(generator), chunk(generator, glm::i64vec3(0, 0, 0)) {

    this->shader = gl::Shader::create(R"(
        #version 330 core

        layout (location = 0) in vec3 vert_pos;
        layout (location = 1) in vec3 vert_normal;
        layout (location = 2) in vec3 vert_color;

        uniform mat4 mvp;
        
        out vec3 frag_normal;
        out vec3 frag_color;

        void main() {
            gl_Position = mvp * vec4(vert_pos, 1.0f);
            frag_normal = vert_normal;
            frag_color = vert_color;
        }

    )", R"(
        #version 330 core

        in vec3 frag_normal;
        in vec3 frag_color;

        out vec4 color;

        void main() {
            color = vec4(frag_color, 1.0f);
        }

    )").unwrap();
    
    chunk.load();
}

Terrain::~Terrain() {

}

void Terrain::update(const ui::Camera& camera) {
    int render_distance = int(std::ceil(camera.get_z_far() / this->generator.get_chunk_size()));

    
}

void Terrain::draw(const ui::Camera& camera) {
    this->shader.bind();
    auto index = this->shader.get_uniform_location("mvp").unwrap();
    auto mvp = camera.get_projection() * camera.get_view();
    glUniformMatrix4fv(index, 1, GL_FALSE, &mvp[0][0]);
    chunk.draw();
}
