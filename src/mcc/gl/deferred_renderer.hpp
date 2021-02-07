#pragma once

#include <string>
#include <functional>
#include <glm/glm.hpp>

#include <mcc/result.hpp>
#include <mcc/gl/shader.hpp>
#include <mcc/gl/vertex_array.hpp>
#include <mcc/gl/vertex_buffer.hpp>
#include <mcc/ui/camera.hpp>

namespace mcc::gl {
    /*
        Deferred renderer wrapper.
    */
    class DeferredRenderer final {
    public:
        DeferredRenderer();
        DeferredRenderer(DeferredRenderer&& rhs);
        DeferredRenderer& operator=(DeferredRenderer&& rhs);
        ~DeferredRenderer();

        static Result<DeferredRenderer, std::string> create(int width, int height);

        void resize(int width, int height);
        void render(float dt, const ui::Camera& camera, const std::function<void()>& draw_opaque, const std::function<void()>& draw_transparent);

        inline void set_debug_rendering(bool debug_rendering) { this->debug_rendering = debug_rendering; }
        inline void set_wireframe(bool wireframe) { this->wireframe = wireframe; }
        inline void set_sky_color(const glm::vec3& sky_color) { this->sky_color = sky_color; }

    private:
        int width, height;
        bool wireframe;
        bool debug_rendering;
        glm::vec3 sky_color;

        Shader ss_shader;

        struct {
            unsigned int fbo;
            unsigned int albedo, position, normal, depth;
        } gbuffer;

        struct {
            unsigned int fbo;
            unsigned int color_buffer;
            unsigned int noise;
            std::vector<glm::vec3> samples;
            Shader shader;
        } ssao;

        struct {
            unsigned int fbo;
            unsigned int color_buffer;
            Shader shader;
        } ssao_blur;

        struct {
            VertexBuffer vb;
            VertexArray va;
        } ss_quad;
    };
}