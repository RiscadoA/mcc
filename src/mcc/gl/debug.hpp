#pragma once

#include <mcc/gl/shader.hpp>
#include <mcc/gl/vertex_array.hpp>
#include <mcc/gl/vertex_buffer.hpp>
#include <glm/glm.hpp>

#include <vector>
#include <mutex>

namespace mcc::gl {
    class Debug {
    public:
        static void init();
        static void terminate();
        static void flush(const glm::mat4& vp, float dt);

        static void draw_box(glm::vec3 position, glm::vec3 scale, glm::vec4 color, float time = 0.0f);
        static void draw_sphere(glm::vec3 position, float radius, glm::vec4 color, float time = 0.0f);

    private:
        static void add_command(const glm::mat4& transform, glm::vec4 color, unsigned int first, unsigned int count, float time);

        struct Command {
            glm::mat4 transform;
            glm::vec4 color;
            unsigned int first, count;
            float life;
        };

        static bool initialized;
        static std::vector<Command> commands;
        static std::mutex command_mutex;

        static Shader shader;
        static VertexBuffer vb;
        static VertexArray va;

        static unsigned int color_loc, vp_loc, model_loc;
        static unsigned int box_first, box_count;
        static unsigned int sphere_first, sphere_count;
    };
}