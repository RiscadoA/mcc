#include <mcc/config.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <mcc/gl/shader.hpp>
#include <mcc/gl/vertex_array.hpp>
#include <mcc/ui/camera.hpp>

#include <mcc/map/terrain.hpp>

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

int main(int argc, char** argv) try {
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
        glm::vec3(0.0f, 0.5f, -1.0f),
        glm::vec2(0.0f, 0.0f)
    );

    glfwSetCursorPosCallback(win, glfw_cursor_pos_callback);
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(1);

    // Load game
    auto registry = mcc::map::Block::Registry();
    auto terrain = mcc::map::Terrain(mcc::map::Generation(0), registry);
    auto view_distance = int(config["camera.view_distance"].unwrap().as_integer().unwrap());

    // Prepare rendering
    glEnable(GL_DEPTH_TEST);
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
        camera->update();

        glClearColor(0.1f, 0.5f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        terrain.update(dt);
        
        glm::i64vec3 chunk_pos = glm::round(camera->get_position() / glm::vec3(mcc::map::Chunk::Size * mcc::map::Block::Size));
        
        /*auto& chunk = terrain.reference(glm::i64vec3(0, -1, 0), 2.5f);
        terrain.dereference(chunk);*/

        for (int x = -view_distance; x <= view_distance; ++x) {
            for (int y = -view_distance; y <= view_distance; ++y) {
                for (int z = -view_distance; z <= view_distance; ++z) {
                    auto& chunk = terrain.reference(chunk_pos + glm::i64vec3(x, y, z), 2.5f);
                    terrain.dereference(chunk);
                }
            }
        }

        glm::mat4 vp = camera->get_projection() * camera->get_view();
        terrain.draw(vp);

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