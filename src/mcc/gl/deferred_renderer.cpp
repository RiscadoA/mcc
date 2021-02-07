#include <mcc/gl/deferred_renderer.hpp>
#include <mcc/gl/debug.hpp>

#include <vector>
#include <sstream>
#include <random>

#include <GL/glew.h>

using namespace mcc;
using namespace mcc::gl;

mcc::gl::DeferredRenderer::DeferredRenderer() {
    this->width = this->height = 0;
    this->wireframe = false;
    this->sky_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    this->debug_rendering = false;

    this->gbuffer.fbo = 0;
    this->gbuffer.albedo = 0;
    this->gbuffer.position = 0;
    this->gbuffer.normal = 0;
    this->gbuffer.depth = 0;

    this->ssao.fbo = 0;
    this->ssao.color_buffer = 0;
    this->ssao.noise = 0;

    this->ssao_blur.fbo = 0;
    this->ssao_blur.color_buffer = 0;
}

mcc::gl::DeferredRenderer::DeferredRenderer(DeferredRenderer&& rhs) {
    *this = std::move(rhs);
}

DeferredRenderer& mcc::gl::DeferredRenderer::operator=(DeferredRenderer&& rhs) {
    this->width = rhs.width;
    this->height = rhs.height;
    this->wireframe = rhs.wireframe;
    this->sky_color = rhs.sky_color;
    this->debug_rendering = rhs.debug_rendering;

    this->gbuffer.fbo = rhs.gbuffer.fbo;
    this->gbuffer.albedo = rhs.gbuffer.albedo;
    this->gbuffer.position = rhs.gbuffer.position;
    this->gbuffer.normal = rhs.gbuffer.normal;
    this->gbuffer.depth = rhs.gbuffer.depth;
    rhs.gbuffer.fbo = 0;
    rhs.gbuffer.albedo = 0;
    rhs.gbuffer.position = 0;
    rhs.gbuffer.normal = 0;
    rhs.gbuffer.depth = 0;

    this->ssao.fbo = rhs.ssao.fbo;
    this->ssao.color_buffer = rhs.ssao.color_buffer;
    this->ssao.noise = rhs.ssao.noise;
    this->ssao.shader = std::move(rhs.ssao.shader);
    this->ssao.samples = std::move(rhs.ssao.samples);
    rhs.ssao.fbo = 0;
    rhs.ssao.color_buffer = 0;
    rhs.ssao.noise = 0;

    this->ssao_blur.fbo = rhs.ssao_blur.fbo;
    this->ssao_blur.color_buffer = rhs.ssao_blur.color_buffer;
    this->ssao_blur.shader = std::move(rhs.ssao_blur.shader);
    rhs.ssao_blur.fbo = 0;
    rhs.ssao_blur.color_buffer = 0;

    this->ss_shader = std::move(rhs.ss_shader);
    this->ss_quad = std::move(rhs.ss_quad);

    return *this;
}

mcc::gl::DeferredRenderer::~DeferredRenderer() {
    if (this->gbuffer.fbo != 0) {
        glDeleteFramebuffers(1, &this->gbuffer.fbo);
    }

    if (this->gbuffer.albedo != 0) {
        glDeleteTextures(1, &this->gbuffer.albedo);
    }

    if (this->gbuffer.position != 0) {
        glDeleteTextures(1, &this->gbuffer.position);
    }

    if (this->gbuffer.normal != 0) {
        glDeleteTextures(1, &this->gbuffer.normal);
    }

    if (this->gbuffer.depth != 0) {
        glDeleteTextures(1, &this->gbuffer.depth);
    }

    if (this->ssao.fbo != 0) {
        glDeleteFramebuffers(1, &this->ssao.fbo);
    }

    if (this->ssao.color_buffer != 0) {
        glDeleteTextures(1, &this->ssao.color_buffer);
    }

    if (this->ssao.noise != 0) {
        glDeleteTextures(1, &this->ssao.noise);
    }

    if (this->ssao_blur.fbo != 0) {
        glDeleteFramebuffers(1, &this->ssao_blur.fbo);
    }

    if (this->ssao_blur.color_buffer != 0) {
        glDeleteTextures(1, &this->ssao_blur.color_buffer);
    }
}

Result<DeferredRenderer, std::string> mcc::gl::DeferredRenderer::create(int width, int height) {
    auto renderer = DeferredRenderer();

    renderer.width = width;
    renderer.height = height;

    // Prepare screen space quad
    float ss_quad_data[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
        +1.0f, -1.0f, 1.0f, 0.0f,
        +1.0f, +1.0f, 1.0f, 1.0f,
        +1.0f, +1.0f, 1.0f, 1.0f,
        -1.0f, +1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
    };

    renderer.ss_quad.vb = mcc::gl::VertexBuffer::create(sizeof(ss_quad_data), ss_quad_data, mcc::gl::Usage::Static).unwrap();
    renderer.ss_quad.va = mcc::gl::VertexArray::create({
        mcc::gl::Attribute(
            renderer.ss_quad.vb,
            sizeof(float) * 4, 0 * sizeof(float),
            2, mcc::gl::Attribute::Type::F32,
            0
        ),
        mcc::gl::Attribute(
            renderer.ss_quad.vb,
            sizeof(float) * 4, 2 * sizeof(float),
            2, mcc::gl::Attribute::Type::F32,
            1
        )
    }).unwrap();

    // Prepare GBuffer framebuffer and textures
    glGenFramebuffers(1, &renderer.gbuffer.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, renderer.gbuffer.fbo);

    glGenTextures(1, &renderer.gbuffer.albedo);
    glBindTexture(GL_TEXTURE_2D, renderer.gbuffer.albedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, renderer.width, renderer.height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer.gbuffer.albedo, 0);

    glGenTextures(1, &renderer.gbuffer.position);
    glBindTexture(GL_TEXTURE_2D, renderer.gbuffer.position);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, renderer.width, renderer.height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, renderer.gbuffer.position, 0);

    glGenTextures(1, &renderer.gbuffer.normal);
    glBindTexture(GL_TEXTURE_2D, renderer.gbuffer.normal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, renderer.width, renderer.height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, renderer.gbuffer.normal, 0);

    glGenTextures(1, &renderer.gbuffer.depth);
    glBindTexture(GL_TEXTURE_2D, renderer.gbuffer.depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, renderer.width, renderer.height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, renderer.gbuffer.depth, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glDeleteFramebuffers(1, &renderer.gbuffer.fbo);
        renderer.gbuffer.fbo = 0;

        std::stringstream ss;
        ss << "mcc::gl::DeferredRenderer::create() failed:" << std::endl;
        ss << "GBuffer framebuffer is not complete" << std::endl;
        ss << "glCheckFramebufferStatus() didn't return GL_FRAMEBUFFER_COMPLETE";
        return Result<DeferredRenderer, std::string>::error(ss.str());
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // SSAO framebuffer and textures
    glGenFramebuffers(1, &renderer.ssao.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, renderer.ssao.fbo);

    glGenTextures(1, &renderer.ssao.color_buffer);
    glBindTexture(GL_TEXTURE_2D, renderer.ssao.color_buffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, renderer.width, renderer.height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer.ssao.color_buffer, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glDeleteFramebuffers(1, &renderer.ssao.fbo);
        renderer.ssao.fbo = 0;

        std::stringstream ss;
        ss << "mcc::gl::DeferredRenderer::create() failed:" << std::endl;
        ss << "SSAO framebuffer is not complete" << std::endl;
        ss << "glCheckFramebufferStatus() didn't return GL_FRAMEBUFFER_COMPLETE";
        return Result<DeferredRenderer, std::string>::error(ss.str());
    }

    // SSAO kernel
    std::uniform_real_distribution<float> random_floats(0.0f, 1.0f);
    std::default_random_engine generator;
    renderer.ssao.samples.resize(64);
    for (int i = 0; i < renderer.ssao.samples.size(); ++i) {
        auto sample = glm::vec3(
            random_floats(generator) * 2.0 - 1.0,
            random_floats(generator) * 2.0 - 1.0,
            random_floats(generator)
        );
        sample = glm::normalize(sample) * random_floats(generator);
        
        float scale = (float)i / (float)renderer.ssao.samples.size();
        scale = 0.1f + scale * scale * (1.0f - 0.1f);
        sample *= scale;

        renderer.ssao.samples[i] = sample;
    }

    // SSAO noise texture
    std::vector<glm::vec3> ssao_noise;
    for (int i = 0; i < 16; ++i) {
        ssao_noise.push_back({
            random_floats(generator) * 2.0 - 1.0,
            random_floats(generator) * 2.0 - 1.0,
            0.0f
        });
    }

    glGenTextures(1, &renderer.ssao.noise);
    glBindTexture(GL_TEXTURE_2D, renderer.ssao.noise);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssao_noise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // SSAO blur framebuffer and texture
    glGenFramebuffers(1, &renderer.ssao_blur.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, renderer.ssao_blur.fbo);

    glGenTextures(1, &renderer.ssao_blur.color_buffer);
    glBindTexture(GL_TEXTURE_2D, renderer.ssao_blur.color_buffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, renderer.width, renderer.height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderer.ssao_blur.color_buffer, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glDeleteFramebuffers(1, &renderer.ssao_blur.fbo);
        renderer.ssao_blur.fbo = 0;

        std::stringstream ss;
        ss << "mcc::gl::DeferredRenderer::create() failed:" << std::endl;
        ss << "SSAO Blur framebuffer is not complete" << std::endl;
        ss << "glCheckFramebufferStatus() didn't return GL_FRAMEBUFFER_COMPLETE";
        return Result<DeferredRenderer, std::string>::error(ss.str());
    }

    // SSAO shader
    renderer.ssao.shader = Shader::create(R"(
        #version 330 core

        layout (location = 0) in vec2 vert_pos;
        layout (location = 1) in vec2 vert_uv;

        out vec2 frag_uv;

        void main() {
            gl_Position = vec4(vert_pos, 0.0f, 1.0f);
            frag_uv = vert_uv;            
        }
    )", R"(
        #version 330 core

        in vec2 frag_uv;

        out float frag_color;
        
        const int NUM_SAMPLES = 64;

        uniform sampler2D position_tex;
        uniform sampler2D normal_tex;
        uniform sampler2D noise_tex;

        uniform mat4 projection;
        uniform vec2 noise_scale;
        uniform vec3 samples[NUM_SAMPLES];

        const float radius = 0.5;
        const float bias = 0.025;
        const float magnitude = 1.1;
        const float constrast = 1.1;

        void main() {
            vec3 frag_pos = texture(position_tex, frag_uv).xyz;
            vec3 normal = normalize(texture(normal_tex, frag_uv).xyz);
            vec3 random_vec = normalize(texture(noise_tex, frag_uv * noise_scale).xyz);

            vec3 tangent = normalize(random_vec - normal * dot(random_vec, normal));
            vec3 bitangent = cross(normal, tangent);
            mat3 tbn = mat3(tangent, bitangent, normal);
            
            float occlusion = 0.0;
            for (int i = 0; i < NUM_SAMPLES; ++i) {
                vec3 sample_pos = tbn * samples[i];
                sample_pos = frag_pos + sample_pos * radius;
                
                vec4 offset = projection * vec4(sample_pos, 1.0);
                offset.xy /= offset.w;
                offset.xy = offset.xy * 0.5 + 0.5;
                
                float sample_depth = texture(position_tex, offset.xy).z;
                float range_check = smoothstep(0.0, 1.0, radius / abs(frag_pos.z - sample_depth));
                occlusion += (sample_depth >= sample_pos.z + bias ? 1.0 : 0.0) * range_check;
            }
            
            occlusion = 1.0 - occlusion / NUM_SAMPLES;
            occlusion = pow(occlusion, magnitude);
            frag_color = constrast * (occlusion - 0.5) + 0.5;
        }
    )").unwrap();

    // SSAO Blur shader
    renderer.ssao_blur.shader = Shader::create(R"(
        #version 330 core

        layout (location = 0) in vec2 vert_pos;
        layout (location = 1) in vec2 vert_uv;

        out vec2 frag_uv;

        void main() {
            gl_Position = vec4(vert_pos, 0.0f, 1.0f);
            frag_uv = vert_uv;            
        }
    )", R"(
        #version 330 core

        in vec2 frag_uv;

        out float frag_color;

        uniform sampler2D ssao_tex;

        void main() {
            vec2 texel_size = 1.0 / vec2(textureSize(ssao_tex, 0));
            float result = 0.0;
            for (int x = -2; x < 2; ++x) {
                for (int y = -2; y < 2; ++y) {
                    vec2 offset = vec2(float(x), float(y)) * texel_size;
                    result += texture(ssao_tex, frag_uv + offset).r;
                }
            }
            frag_color = result / (4.0 * 4.0);
        }
    )").unwrap();

    // Prepare screen space shader
    renderer.ss_shader = Shader::create(R"(
        #version 330 core

        layout (location = 0) in vec2 vert_pos;
        layout (location = 1) in vec2 vert_uv;

        out vec2 frag_uv;

        void main() {
            gl_Position = vec4(vert_pos, 0.0f, 1.0f);
            frag_uv = vert_uv;
        }
    )", R"(
        #version 330 core

        #define PI 3.1415926535897932384626433832795

        in vec2 frag_uv;

        out vec4 frag_color;

        uniform sampler2D albedo_tex;
        uniform sampler2D position_tex;
        uniform sampler2D normal_tex;
        uniform sampler2D ssao_tex;

        uniform vec3 sky_color;
        uniform float z_far;

        uniform mat4 view;

        const vec3 world_light_dir = normalize(vec3(-0.7, 1.5, 0.5));

        void main() {
            vec3 albedo = texture(albedo_tex, frag_uv).rgb;
            vec3 position = texture(position_tex, frag_uv).xyz;
            vec3 normal = texture(normal_tex, frag_uv).xyz;
            float ambient_occlusion = texture(ssao_tex, frag_uv).r;

            vec3 lighting = albedo * ambient_occlusion * 0.3f;
            vec3 light_dir = normalize(mat3(view) * world_light_dir);
            
            vec3 diffuse = max(dot(normal, light_dir), 0.0f) * albedo * ambient_occlusion;
            lighting += diffuse;

            if (normal.x == 0.0f && normal.y == 0.0f && normal.z == 0.0f) {
                // Sky
                lighting = albedo;
            }

            // Fog
            float depth = min(1.0f, length(position) / z_far);
            float fog = depth * depth;
            frag_color = vec4(mix(lighting, sky_color, fog), 1.0);
        }
    )").unwrap();

    return Result<DeferredRenderer, std::string>::success(std::move(renderer));
}

void mcc::gl::DeferredRenderer::resize(int width, int height) {

}

void mcc::gl::DeferredRenderer::render(float dt, const ui::Camera& camera, const std::function<void()>& draw_opaque, const std::function<void()>& draw_transparent) {
    auto view = camera.get_view();
    auto proj = camera.get_projection();
    auto vp = proj * view;
    
    // Opaque pass
    glBindFramebuffer(GL_FRAMEBUFFER, this->gbuffer.fbo);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glPolygonMode(GL_FRONT_AND_BACK, this->wireframe ? GL_LINE : GL_FILL);

    // Clear framebuffer
    GLuint draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(1, &draw_buffers[0]);
    glClearColor(sky_color.r, sky_color.g, sky_color.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawBuffers(2, &draw_buffers[1]);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawBuffers(3, &draw_buffers[0]);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Draw opaque objects
    draw_opaque();

    // SSAO pass
    ss_quad.va.bind();
    
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    glBindFramebuffer(GL_FRAMEBUFFER, this->ssao.fbo);
    glDrawBuffers(1, &draw_buffers[0]);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->gbuffer.position);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, this->gbuffer.normal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, this->ssao.noise);

    auto noise_scale = glm::vec2(float(this->width) / 4.0f, float(this->height) / 4.0f);

    this->ssao.shader.bind();
    glUniform1i(this->ssao.shader.get_uniform_location("position_tex").unwrap(), 0);
    glUniform1i(this->ssao.shader.get_uniform_location("normal_tex").unwrap(), 1);
    glUniform1i(this->ssao.shader.get_uniform_location("noise_tex").unwrap(), 2);
    glUniform3fv(this->ssao.shader.get_uniform_location("samples").unwrap(), this->ssao.samples.size(), &this->ssao.samples[0][0]);
    glUniform2fv(this->ssao.shader.get_uniform_location("noise_scale").unwrap(), 1, &noise_scale[0]);
    glUniformMatrix4fv(this->ssao.shader.get_uniform_location("projection").unwrap(), 1, GL_FALSE, &proj[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // SSAO Blur pass
    glBindFramebuffer(GL_FRAMEBUFFER, this->ssao_blur.fbo);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->ssao.color_buffer);

    this->ssao_blur.shader.bind();
    glUniform1i(this->ssao_blur.shader.get_uniform_location("ssao_tex").unwrap(), 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Screen quad rendering
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->gbuffer.albedo);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, this->gbuffer.position);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, this->gbuffer.normal);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, this->ssao_blur.color_buffer);

    this->ss_shader.bind();
    glUniform1i(this->ss_shader.get_uniform_location("albedo_tex").unwrap(), 0);
    glUniform1i(this->ss_shader.get_uniform_location("position_tex").unwrap(), 1);
    glUniform1i(this->ss_shader.get_uniform_location("normal_tex").unwrap(), 2);
    glUniform1i(this->ss_shader.get_uniform_location("ssao_tex").unwrap(), 3);
    glUniform3f(this->ss_shader.get_uniform_location("sky_color").unwrap(), this->sky_color.r, this->sky_color.g, this->sky_color.b);
    glUniform1f(this->ss_shader.get_uniform_location("z_far").unwrap(), camera.get_z_far());
    glUniformMatrix4fv(this->ss_shader.get_uniform_location("view").unwrap(), 1, GL_FALSE, &view[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Debug draw on top of screen
    glBindFramebuffer(GL_READ_FRAMEBUFFER, this->gbuffer.fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, this->width, this->height, 0, 0, this->width, this->height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    if (debug_rendering) {
        mcc::gl::Debug::flush(vp, 1 / 144.0f);
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, this->gbuffer.fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    /*glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBlitFramebuffer(0, 0, this->width, this->height, 0, 0, this->width / 2, this->height / 2, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    glBlitFramebuffer(0, 0, this->width, this->height, this->width / 2, 0, this->width, this->height / 2, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glReadBuffer(GL_COLOR_ATTACHMENT2);
    glBlitFramebuffer(0, 0, this->width, this->height, 0, this->height / 2, this->width / 2, this->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, this->ssao.fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBlitFramebuffer(0, 0, this->width, this->height, this->width / 2, this->height / 2, this->width, this->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    */

    /*glBindFramebuffer(GL_READ_FRAMEBUFFER, this->ssao_blur.fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBlitFramebuffer(0, 0, this->width, this->height, 0, 0, this->width, this->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);*/

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
