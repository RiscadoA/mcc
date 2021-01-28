#include <mcc/config.hpp>

#include <mcc/data/manager.hpp>
#include <mcc/data/model.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <mcc/gl/shader.hpp>
#include <mcc/gl/vertex_array.hpp>
#include <mcc/gl/mesh.hpp>
#include <mcc/ui/camera.hpp>

#include <iostream>
#include <chrono>
#include <random>

mcc::ui::Camera* camera;
float dt = 1.0f / 60.0f;
float camera_sensitivity = 0.1f;
float camera_speed = 1.0f;
const glm::vec4 sky_color = { 0.1f, 0.5f, 0.8f, 1.0f };
bool wireframe = false;

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
    if (action == GLFW_PRESS && key == GLFW_KEY_F1) {
        wireframe = !wireframe;
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

int main(int argc, char** argv) try {
    auto config = mcc::Config(argc, argv);

    camera_sensitivity = float(config["camera.sensitivity"].unwrap().as_double().unwrap());
    camera_speed = float(config["camera.speed"].unwrap().as_double().unwrap());
    
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW:\nglfwInit() didn't return GLFW_TRUE\n";
        std::abort();
    }

#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    auto win = glfwCreateWindow(
        int(config["window.width"].unwrap().as_integer().unwrap()),
        int(config["window.height"].unwrap().as_integer().unwrap()),
        "MCC",
        nullptr, //glfwGetPrimaryMonitor(),
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

    // Setup asset manager
    auto model_loader = mcc::data::Model::Loader(config);
    auto manager = mcc::data::Manager(config, { { "model", &model_loader } });

    // Setup camera
    camera = new mcc::ui::Camera(
        float(glm::radians(config["camera.fov"].unwrap().as_double().unwrap())),
        float(config["window.width"].unwrap().as_double().unwrap() / config["window.height"].unwrap().as_double().unwrap()),
        float(config["camera.z_near"].unwrap().as_double().unwrap()),
        float(config["camera.z_far"].unwrap().as_double().unwrap()),
        glm::vec3(0.0f, 0.0f, -10.0f),
        glm::vec2(0.0f, 0.0f)
    );

    glfwSetCursorPosCallback(win, glfw_cursor_pos_callback);
    glfwSetKeyCallback(win, glfw_key_callback);
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(1);
  
    // Prepare textures and framebuffer for GBuffer
    GLuint ss_fbo;
    glGenFramebuffers(1, &ss_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, ss_fbo);
    
    GLuint ss_albedo, ss_position, ss_normal, ss_depth;
    glGenTextures(1, &ss_albedo);
    glBindTexture(GL_TEXTURE_2D, ss_albedo);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB,
        int(config["window.width"].unwrap().as_integer().unwrap()),
        int(config["window.height"].unwrap().as_integer().unwrap()),
        0, GL_RGB, GL_UNSIGNED_BYTE, nullptr
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ss_albedo, 0);

    glGenTextures(1, &ss_position);
    glBindTexture(GL_TEXTURE_2D, ss_position);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB16F,
        int(config["window.width"].unwrap().as_integer().unwrap()),
        int(config["window.height"].unwrap().as_integer().unwrap()),
        0, GL_RGB, GL_FLOAT, nullptr
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, ss_position, 0);
    
    glGenTextures(1, &ss_normal);
    glBindTexture(GL_TEXTURE_2D, ss_normal);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB,
        int(config["window.width"].unwrap().as_integer().unwrap()),
        int(config["window.height"].unwrap().as_integer().unwrap()),
        0, GL_RGB, GL_FLOAT, nullptr
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, ss_normal, 0);

    glGenTextures(1, &ss_depth);
    glBindTexture(GL_TEXTURE_2D, ss_depth);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        int(config["window.width"].unwrap().as_integer().unwrap()),
        int(config["window.height"].unwrap().as_integer().unwrap()),
        0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ss_depth, 0);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::abort();
    }

    // Prepare mesh shader
    auto mesh_shader = mcc::gl::Shader::create(R"(
        #version 330 core

        layout (location = 0) in vec3 vert_pos;
        layout (location = 1) in vec3 vert_normal;
        layout (location = 2) in vec4 vert_color;

        uniform mat4 model;
        uniform mat4 vp;
        
        out vec3 frag_albedo;
        out vec3 frag_pos;
        out vec3 frag_normal;

        void main() {
            frag_pos = (model * vec4(vert_pos, 1.0f)).xyz;
            gl_Position = vp * vec4(frag_pos, 1.0f);
            frag_albedo = vert_color.rgb;
            frag_normal = vert_normal;
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
            normal = frag_normal;
        }

    )").unwrap();

    // Prepare screen space shader
    auto ss_shader = mcc::gl::Shader::create(R"(
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

        uniform vec3 sky_color;
        uniform vec3 camera_position;
        uniform float z_far;

        const vec3 light_dir = normalize(vec3(-0.7, 1.0, -0.5));

        void main() {
            vec3 albedo = texture(albedo_tex, frag_uv).rgb;
            vec3 position = texture(position_tex, frag_uv).xyz;
            vec3 normal = texture(normal_tex, frag_uv).xyz;

            vec3 lighting = albedo * 0.4f;
            vec3 diffuse = max(dot(normal, light_dir), 0.0f) * albedo;
            lighting += diffuse;

            if (normal.x == 0.0f && normal.y == 0.0f && normal.z == 0.0f) {
                lighting = albedo;
            }

            float depth = min(1.0f, distance(position, camera_position) / z_far);
            depth = min(1.0f, atanh(depth) / 1.5);
            frag_color = vec4(mix(lighting, sky_color, depth), 1.0);
        }
    )").unwrap();

    // Prepare screen space quad
    struct {
        mcc::gl::VertexArray va;
        mcc::gl::VertexBuffer vb;
    } ss_quad;

    {
        float data[] = {
            -1.0f, -1.0f, 0.0f, 0.0f,
            +1.0f, -1.0f, 1.0f, 0.0f,
            +1.0f, +1.0f, 1.0f, 1.0f,
            +1.0f, +1.0f, 1.0f, 1.0f,
            -1.0f, +1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
        };

        ss_quad.vb = mcc::gl::VertexBuffer::create(sizeof(data), data, mcc::gl::Usage::Static).unwrap();
        ss_quad.va = mcc::gl::VertexArray::create({
            mcc::gl::Attribute(
                ss_quad.vb,
                sizeof(float) * 4, 0 * sizeof(float),
                2, mcc::gl::Attribute::Type::F32,
                0
            ),
            mcc::gl::Attribute(
                ss_quad.vb,
                sizeof(float) * 4, 2 * sizeof(float),
                2, mcc::gl::Attribute::Type::F32,
                1
            )        
        }).unwrap();
    }

    auto model_loc = mesh_shader.get_uniform_location("model").unwrap();
    auto vp_loc = mesh_shader.get_uniform_location("vp").unwrap();

    auto voxel_model = manager.get<mcc::data::Model>("model.monu10").unwrap();

    auto& matrix = voxel_model->get_matrix();
    auto octree = mcc::gl::matrix_to_octree(matrix);
    auto mesh = mcc::gl::Mesh();

    int lod = 7;
    bool stop = false;
    mesh.update(octree, 128.0f, lod);

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

        if (!stop && glfwGetKey(win, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
            stop = true;
            lod += 1;
            mesh.update(octree, 128.0f, lod);
        }
        else if (glfwGetKey(win, GLFW_KEY_KP_ADD) == GLFW_RELEASE && glfwGetKey(win, GLFW_KEY_KP_SUBTRACT) == GLFW_RELEASE) {
            stop = false;
        }
        else if (!stop && glfwGetKey(win, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
            stop = true;
            lod -= 1;
            mesh.update(octree, 128.0f, lod);
        }

        camera->update();

        glBindFramebuffer(GL_FRAMEBUFFER, ss_fbo);
        glEnable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);

        GLuint draw_buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(1, &draw_buffers[0]);
        glClearColor(sky_color.r, sky_color.g, sky_color.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawBuffers(2, &draw_buffers[1]);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawBuffers(3, &draw_buffers[0]);
        glClear(GL_DEPTH_BUFFER_BIT);

        mesh_shader.bind();
        
        glm::mat4 vp = camera->get_projection() * camera->get_view();
        glm::mat4 model = glm::mat4(1.0f);
        //model = glm::rotate(model, glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        glUniformMatrix4fv(model_loc, 1, GL_FALSE, &model[0][0]);
        glUniformMatrix4fv(vp_loc, 1, GL_FALSE, &vp[0][0]);
        mesh.draw_opaque();

        // Screen quad rendering
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        ss_shader.bind();
        glUniform1i(ss_shader.get_uniform_location("albedo_tex").unwrap(), 0);
        glUniform1i(ss_shader.get_uniform_location("position_tex").unwrap(), 1);
        glUniform1i(ss_shader.get_uniform_location("normal_tex").unwrap(), 2);
        glUniform3f(ss_shader.get_uniform_location("sky_color").unwrap(), sky_color.r, sky_color.g, sky_color.b);  
        glUniform3f(ss_shader.get_uniform_location("camera_position").unwrap(), camera->get_position().x, camera->get_position().y, camera->get_position().z);
        glUniform1f(ss_shader.get_uniform_location("z_far").unwrap(), float(config["camera.z_far"].unwrap().as_double().unwrap()));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ss_albedo);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, ss_position);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, ss_normal);
        ss_quad.va.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(win);
    }

    // Unload game

    delete camera;

    glfwDestroyWindow(win);

    return 0;
} catch(std::exception& e)  {
    std::cerr << "Caught exception on main():\n" << e.what() << "\n";
    std::abort();
} catch (...) {
    std::cerr << "Caught unknown exception on main()\n";
    std::abort();
}