#include <mcc/ui/camera.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace mcc;
using namespace mcc::ui;

Camera::Camera(float fov, float aspect_ratio, float z_near, float z_far, glm::vec3 pos, glm::vec2 rot) :
    fov(fov), aspect_ratio(aspect_ratio), z_near(z_near), z_far(z_far), pos(pos), rot(rot) {
    this->update();
}

void Camera::move(const glm::vec3& translation) {
    this->pos += translation;
}

void Camera::rotate(const glm::vec2& rotation) {
    this->set_rotation(this->rot + rotation);
}

void Camera::set_position(const glm::vec3& position) {
    this->pos = position;
}

void Camera::set_rotation(const glm::vec2& rotation) {
    this->rot = rotation;
    this->rot.x = glm::clamp(this->rot.x, -glm::pi<float>() / 2 + 0.01f, glm::pi<float>() / 2 - 0.01f);
    this->rot.y = glm::mod(this->rot.y, 2 * glm::pi<float>());
}

void Camera::set_fov(float fov) {
    this->fov = fov;
}

void Camera::set_aspect_ratio(float aspect_ratio) {
    this->aspect_ratio = aspect_ratio;
}

void Camera::set_z_near(float z_near) {
    this->z_near = z_near;
}

void Camera::set_z_far(float z_far) {
    this->z_far = z_far;
}

void Camera::update() {
    // Build direction from rotation
    this->forward = glm::normalize(glm::vec3(
        glm::sin(this->rot.y) * glm::cos(this->rot.x),
        glm::sin(this->rot.x),
        glm::cos(this->rot.y) * glm::cos(this->rot.x)
    ));
    this->right = glm::normalize(glm::cross(this->forward, this->get_world_up()));
    this->up = glm::normalize(-glm::cross(this->forward, this->right));
    
    this->proj = glm::perspective(this->fov, this->aspect_ratio, this->z_near, this->z_far);
    this->view = glm::lookAt(this->pos, this->pos + this->forward, this->get_world_up());
}
