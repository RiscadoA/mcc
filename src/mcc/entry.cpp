#include <mcc/config.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <mcc/gl/shader.hpp>
#include <mcc/gl/vertex_array.hpp>
#include <mcc/ui/camera.hpp>

#include <mcc/map/terrain.hpp>

#include <iostream>
#include <chrono>

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
    /*if(id == 131169 || id == 131185 || id == 131218 || id == 131204) {
        return;
    }*/

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
        glfwGetPrimaryMonitor(),
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

    // Setup camera
    camera = new mcc::ui::Camera(
        float(glm::radians(config["camera.fov"].unwrap().as_double().unwrap())),
        float(config["window.width"].unwrap().as_double().unwrap() / config["window.height"].unwrap().as_double().unwrap()),
        float(config["camera.z_near"].unwrap().as_double().unwrap()),
        float(config["camera.z_far"].unwrap().as_double().unwrap()),
        glm::vec3(0.0f, 50.0f, -1.0f),
        glm::vec2(0.0f, 0.0f)
    );

    glfwSetCursorPosCallback(win, glfw_cursor_pos_callback);
    glfwSetKeyCallback(win, glfw_key_callback);
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(1);

    // Load game
    auto generator = mcc::map::Generator(0, 16);
    auto terrain = mcc::map::Terrain(generator);

    // Prepare rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
  
    // Prepare textures and framebuffer
    GLuint ss_diff, ss_depth;
    glGenTextures(1, &ss_diff);
    glBindTexture(GL_TEXTURE_2D, ss_diff);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGB,
        int(config["window.width"].unwrap().as_integer().unwrap()),
        int(config["window.height"].unwrap().as_integer().unwrap()),
        0, GL_RGB, GL_UNSIGNED_BYTE, nullptr
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glGenTextures(1, &ss_depth);
    glBindTexture(GL_TEXTURE_2D, ss_depth);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        int(config["window.width"].unwrap().as_integer().unwrap()),
        int(config["window.height"].unwrap().as_integer().unwrap()),
        0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLuint ss_fbo;
    glGenFramebuffers(1, &ss_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, ss_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ss_diff, 0);  
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ss_depth, 0);  
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::abort();
    }

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

        in vec2 frag_uv;

        out vec4 color;

        uniform vec4 sky_color;
        uniform sampler2D diffuse_tex;
        uniform sampler2D depth_tex;
        uniform float z_near;
        uniform float z_far;

        void main() {
            float depth = 2.0 * texture(depth_tex, frag_uv).x - 1.0;
            depth = 2.0 * z_near * z_far / (z_far + z_near - depth * (z_far - z_near));
            vec4 diffuse = texture(diffuse_tex, frag_uv);
            color = mix(diffuse, sky_color, depth / z_far);
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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

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

        camera->update();

        glBindFramebuffer(GL_FRAMEBUFFER, ss_fbo);
        GLuint draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, &draw_buffers[0]);

        glEnable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
        glClearColor(sky_color.r, sky_color.g, sky_color.b, sky_color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        auto begin = std::chrono::steady_clock::now();
        
        terrain.update(*camera);
        
        auto end = std::chrono::steady_clock::now();
        std::cout << "Terrain update: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms\n";
        begin = end;
        
        terrain.draw(*camera);

        end = std::chrono::steady_clock::now();
        std::cout << "Terrain draw: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        ss_shader.bind();

        glUniform1i(ss_shader.get_uniform_location("diffuse_tex").unwrap(), 0);
        glUniform1i(ss_shader.get_uniform_location("depth_tex").unwrap(), 1);
        glUniform1f(ss_shader.get_uniform_location("z_near").unwrap(), float(config["camera.z_near"].unwrap().as_double().unwrap()));
        glUniform1f(ss_shader.get_uniform_location("z_far").unwrap(), float(config["camera.z_far"].unwrap().as_double().unwrap()));
        glUniform4f(ss_shader.get_uniform_location("sky_color").unwrap(), sky_color.r, sky_color.g, sky_color.b, sky_color.a);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ss_diff);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, ss_depth);

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