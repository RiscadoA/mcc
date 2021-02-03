#pragma once

namespace mcc::map {
    class Generator {
    public:
        virtual unsigned char generate_material(glm::f64vec3 pos, int level) = 0;
    };
}