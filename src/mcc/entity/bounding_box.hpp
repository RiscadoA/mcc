#pragma once

#include <glm/glm.hpp>

namespace mcc::entity {
    // Holds an axis aligned bounding box
    class BoundingBox final {
    public:
        BoundingBox(glm::f32vec3 low, glm::f32vec3 high);
        ~BoundingBox() = default;

        // TO DO

        inline glm::f32vec3 get_low() const { return this->low; }
        inline glm::f32vec3 get_high() const { return this->high; }

    private:
        glm::f32vec3 low, high;
    };
}