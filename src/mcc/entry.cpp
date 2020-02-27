#include <mcc/config.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <mcc/gl/shader.hpp>
#include <mcc/gl/vertex_array.hpp>
#include <mcc/ui/camera.hpp>

#include <iostream>

mcc::ui::Camera* camera;
float dt = 1.0f / 60.0f;
float camera_sensitivity = 0.1f;
float camera_speed = 1.0f;

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

int main(int argc, char** argv) {
    auto config = mcc::Config(argc, argv);

    camera_sensitivity = float(config["camera.sensitivity"].unwrap().as_double().unwrap());
    camera_speed = float(config["camera.speed"].unwrap().as_double().unwrap());
    
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW:\nglfwInit() didn't return GLFW_TRUE\n";
        std::abort();
    }

    auto win = glfwCreateWindow(
        int(config["window.width"].unwrap().as_integer().unwrap()),
        int(config["window.height"].unwrap().as_integer().unwrap()),
        "MCC",
        nullptr,
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

    // Setup camera
    camera = new mcc::ui::Camera(
        float(glm::radians(config["camera.fov"].unwrap().as_double().unwrap())),
        float(config["window.width"].unwrap().as_double().unwrap() / config["window.height"].unwrap().as_double().unwrap()),
        float(config["camera.z_near"].unwrap().as_double().unwrap()),
        float(config["camera.z_far"].unwrap().as_double().unwrap()),
        glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec2(0.0f, 0.0f)
    );

    glfwSetCursorPosCallback(win, glfw_cursor_pos_callback);
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(1);

    // Load game
    
    // OpenGL test
    auto shader = mcc::gl::Shader::create(
        R"(
            #version 330 core

            uniform mat4 mvp;

            in vec3 position;
            in vec3 color;

            out vec4 gl_Position;
            out vec3 frag_color;

            void main() {
                gl_Position = mvp * vec4(position, 1.0);
                frag_color = color;
            }
        )",
        R"(
            #version 330 core

            in vec3 frag_color;

            out vec4 color;

            void main() {
                color = vec4(frag_color, 1.0);
            }
        )"
    ).unwrap();

    float data[36] = {
        -1.0f, -1.0f, 0.0f,       1.0f, 0.0f, 0.0f,
        +1.0f, -1.0f, 0.0f,       0.0f, 1.0f, 0.0f,
        +1.0f, +1.0f, 0.0f,       0.0f, 0.0f, 1.0f,
        +1.0f, +1.0f, 0.0f,       0.0f, 0.0f, 1.0f,
        -1.0f, +1.0f, 0.0f,       1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,       1.0f, 0.0f, 0.0f
    };

    auto vb = mcc::gl::VertexBuffer::create(
        sizeof(data),
        data,
        mcc::gl::Usage::Static
    ).unwrap();

    auto va = mcc::gl::VertexArray::create({
        mcc::gl::Attribute(
            vb,
            6 * sizeof(float), 0 * sizeof(float),
            3, mcc::gl::Attribute::Type::F32,
            shader.get_attribute_location("position").unwrap()
        ),
        mcc::gl::Attribute(
            vb,
            6 * sizeof(float), 3 * sizeof(float),
            3, mcc::gl::Attribute::Type::F32,
            shader.get_attribute_location("color").unwrap()
        )
    }).unwrap();
    
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

        glClearColor(0.1f, 0.5f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glm::mat4 mvp = camera->get_projection() * camera->get_view();

        shader.bind();
        va.bind();
        glUniformMatrix4fv(shader.get_uniform_location("mvp").unwrap(), 1, GL_FALSE, &mvp[0][0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(win);
    }

    // Unload game

    delete camera;
    
    glfwDestroyWindow(win);

    return 0;
}