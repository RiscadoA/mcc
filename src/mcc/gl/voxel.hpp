#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace mcc::gl {
    struct Material {
        glm::u8vec4 color = { 0, 0, 0, 0 }; // RGBA
    };

    struct Octree {
        struct Voxel {
            unsigned char material; // Material index in the palette
            unsigned int child;     // 0 = not subdivided
        };

        Material palette[256];
        std::vector<Voxel> voxels;
    };

    struct Matrix {
        Material palette[256];
        std::vector<unsigned char> voxels;
        glm::u8vec3 size;
        
        Matrix() = default;
        Matrix(Matrix&& rhs) = default;
        Matrix(const Matrix&) = default;
        Matrix& operator=(Matrix&& rhs) = default;
    };

    Octree matrix_to_octree(const Matrix& matrix);
}