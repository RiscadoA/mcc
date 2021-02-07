#include <mcc/config.hpp>

#include <mcc/data/manager.hpp>
#include <mcc/data/model.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/noise.hpp>

#include <mcc/gl/shader.hpp>
#include <mcc/gl/vertex_array.hpp>
#include <mcc/gl/mesh.hpp>
#include <mcc/gl/debug.hpp>
#include <mcc/gl/deferred_renderer.hpp>
#include <mcc/ui/camera.hpp>

#include <mcc/map/chunk.hpp>
#include <mcc/map/generator.hpp>

#include <iostream>
#include <chrono>
#include <random>

mcc::ui::Camera* camera;
float dt = 1.0f / 60.0f;
float camera_sensitivity = 0.1f;
float camera_speed = 1.0f;
const glm::vec4 sky_color = { 0.1f, 0.5f, 0.8f, 1.0f };
bool wireframe = false;
bool debug_rendering = false;

class Generator : public mcc::map::Generator {
public:
    static float simplex(glm::vec3 p, int k) {
        return glm::simplex(p / float(1 << (2 + k)));
    }

    static float simplex(glm::vec2 p, int k) {
        return glm::simplex(p / float(1 << (2 + k)));
    }

    virtual void generate_palette(glm::f64vec3 pos, int level, mcc::gl::Material* palette) override {
        palette[1].color = { 200, 200, 200, 255 };
        for (int i = 2; i < 256; ++i) {
            palette[i].color = { i % 128 + 128, i % 128 + 64, i % 128 + 32, 255 };
        }
    }

    virtual unsigned char generate_material(glm::f64vec3 pos, int level) override {
        /*const float radius = 4000.0f;
        auto projected = glm::normalize(glm::vec3(pos)) * radius;
        auto height = glm::max(radius,
            radius +
            simplex(projected, 7) * 150.0f +
            simplex(projected, 4) * 10.0f +
            simplex(projected, 1) * 1.0f
        );

        if (glm::length(pos) > height) {
            return 0;
        }

        if (height <= radius) {
            return 2;
        }
        else if (height < radius + 50.0f) {
            return 1;
        }
        else {
            return 3;
        }*/

        auto p1 = pos / 10.0;
        auto p2 = pos / 50.0;

        //unsigned char mat = int(abs(glm::round(glm::sin(float(p1.x + p1.y + p1.z)) * 254))) + 1;
        unsigned char mat = 1;
        
        return (glm::cos(float(p2.x)) +
                glm::cos(float(p2.y)) +
                glm::cos(float(p2.z))) < 0 ? mat : 0;
    }
};

void glfw_error_callback(int err, const char* msg) {
    std::cerr << "GLFW error callback called with code '" << err << "':\n" << msg << '\n';
}

void glfw_cursor_pos_callback(GLFWwindow* win, double x, double y) {
    static double px = INFINITY, py;
    if (px != INFINITY) {
        camera->rotate(glm::vec2(
            -(y - py) * camera_sensitivity * dt,
            -(x - px) * camera_sensitivity * dt
        ));       
        camera->update();
    }
    
    px = x;
    py = y;
}

void glfw_key_callback(GLFWwindow* win, int key, int, int action, int) {
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_F1:
            wireframe = !wireframe;
            break;
        case GLFW_KEY_F2:
            debug_rendering = !debug_rendering;
            break;
        }
    }
}

void APIENTRY gl_debug_output(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar *message,
    const void *userParam) {

    // Ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) {
        return;
    }

    std::cerr << "OpenGL debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API" << std::endl; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System" << std::endl; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler" << std::endl; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party" << std::endl; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application" << std::endl; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other" << std::endl; break;
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error" << std::endl; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour" << std::endl; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability" << std::endl; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance" << std::endl; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker" << std::endl; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group" << std::endl; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group" << std::endl; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other" << std::endl; break;
    }
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: High" << std::endl; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: Medium" << std::endl; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: Low" << std::endl; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: Notification" << std::endl; break;
    }

    std::cout << std::endl;
}

int main(int argc, char** argv) {
    auto config = mcc::Config(argc, argv);

    camera_sensitivity = float(config["camera.sensitivity"].unwrap().as_double().unwrap());
    camera_speed = float(config["camera.speed"].unwrap().as_double().unwrap());
    
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW:\nglfwInit() didn't return GLFW_TRUE\n";
        std::abort();
    }

//#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
//#endif
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    int win_width = int(config["window.width"].unwrap().as_integer().unwrap());
    int win_height = int(config["window.height"].unwrap().as_integer().unwrap());

    auto win = glfwCreateWindow(
        win_width,
        win_height,
        "MCC",
        int(config["window.fullscreen"].unwrap().as_integer().unwrap()) == 0 ? nullptr : glfwGetPrimaryMonitor(),
        nullptr
    );

    if (!win) {
        std::cerr << "Failed to create GLFW window:\nglfwCreateWindow() returned nullptr\n";
        std::abort();
    }

    glfwMakeContextCurrent(win);

    auto err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW:\nglewInit() returned '" << err << "':\n" << glewGetErrorString(err) << '\n';
        std::abort();
    }

    GLint flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(gl_debug_output, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    // Setup debug renderer
    mcc::gl::Debug::init();

    // Setup asset manager
    auto model_loader = mcc::data::Model::Loader(config);
    auto manager = mcc::data::Manager(config, { { "model", &model_loader } });

    // Setup camera
    camera = new mcc::ui::Camera(
        float(glm::radians(config["camera.fov"].unwrap().as_double().unwrap())),
        float(win_width) / float(win_height),
        float(config["camera.z_near"].unwrap().as_double().unwrap()),
        float(config["camera.z_far"].unwrap().as_double().unwrap()),
        glm::vec3(0.0f, 0.0f, -20.0f),
        glm::vec2(0.0f, 0.0f)
    );

    glfwSetCursorPosCallback(win, glfw_cursor_pos_callback);
    glfwSetKeyCallback(win, glfw_key_callback);
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(1);
  
    // Prepare renderer
    auto renderer = mcc::gl::DeferredRenderer::create(win_width, win_height).unwrap();
    renderer.set_sky_color({0.0f, 0.5f, 1.0f});

    // Prepare mesh shader
    auto mesh_shader = mcc::gl::Shader::create(R"(
        #version 330 core

        layout (location = 0) in vec3 vert_pos;
        layout (location = 1) in vec3 vert_normal;
        layout (location = 2) in vec4 vert_color;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        out vec3 frag_albedo;
        out vec3 frag_pos;
        out vec3 frag_normal;

        void main() {
            vec4 view_pos = view * model * vec4(vert_pos, 1.0f);
            frag_pos = view_pos.xyz;
            gl_Position = projection * view_pos;
            frag_albedo = vert_color.rgb;
            mat3 normal_matrix = transpose(inverse(mat3(view * model)));
            frag_normal = normal_matrix * vert_normal;
        }
    )", R"(
        #version 330 core

        in vec3 frag_albedo;
        in vec3 frag_pos;
        in vec3 frag_normal;

        layout (location = 0) out vec3 albedo;
        layout (location = 1) out vec3 position;
        layout (location = 2) out vec3 normal;

        void main() {
            albedo = frag_albedo;
            position = frag_pos;
            normal = normalize(frag_normal);
        }
    )").unwrap();

    auto model_loc = mesh_shader.get_uniform_location("model").unwrap();
    auto view_loc = mesh_shader.get_uniform_location("view").unwrap();
    auto projection_loc = mesh_shader.get_uniform_location("projection").unwrap();

    // Setup terrain
    auto generator = Generator();
    auto chunk = mcc::map::Chunk(generator, nullptr, { 0.0, 0.0, 0.0 }, 256.0f, 32, 8);

    auto obj = manager.get<mcc::data::Model>("model.chr_knight").unwrap();

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

    // Main loop
    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();

        // Get input
        if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) {
            camera->move(camera->get_forward() * dt * camera_speed);
        } else if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) {
            camera->move(camera->get_forward() * dt * -camera_speed);
        }
        if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) {
            camera->move(camera->get_right() * dt * camera_speed);
        } else if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) {
            camera->move(camera->get_right() * dt * -camera_speed);
        }
        if (glfwGetKey(win, GLFW_KEY_Q) == GLFW_PRESS) {
            camera->move(camera->get_up() * dt * -camera_speed);
        } else if (glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS) {
            camera->move(camera->get_up() * dt * camera_speed);
        }
        if (glfwGetKey(win, GLFW_KEY_1) == GLFW_PRESS) {
            camera_speed *= 1 - dt;
        }
        else if (glfwGetKey(win, GLFW_KEY_2) == GLFW_PRESS) {
            camera_speed *= 1 + dt;
        }

        camera->update();
        chunk.update(*camera, float(config["camera.lod_multiplier"].unwrap().as_double().unwrap()));

        renderer.render(
            1.0f / 144.0f,
            *camera,
            [&]() {
                // Draw opaque scene
                mesh_shader.bind();
                glUniformMatrix4fv(view_loc, 1, GL_FALSE, &camera->get_view()[0][0]);
                glUniformMatrix4fv(projection_loc, 1, GL_FALSE, &camera->get_projection()[0][0]);

                chunk.draw(*camera, model_loc);

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::rotate(model, glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
                glUniformMatrix4fv(model_loc, 1, GL_FALSE, &model[0][0]);
                obj->get_mesh().draw_opaque();
            },
            [&]() {
                // Draw transparent scene
            }
        );

        glfwSwapBuffers(win);
    }

    // Unload game

    delete camera;

    mcc::gl::Debug::terminate();

    glfwDestroyWindow(win);

    return 0;
}