#include <mcc/gl/voxel.hpp>
#include <functional>
#include <iostream>

using namespace mcc;
using namespace mcc::gl;

Octree mcc::gl::matrix_to_octree(const Matrix& matrix) {
    Octree octree;
    memcpy(octree.palette, matrix.palette, sizeof(octree.palette));

    auto& sz = matrix.size;
    int radius = 1;
    while (radius < glm::max(sz.x, glm::max(sz.y, sz.z))) {
        radius *= 2;
    }

    // Returns the material of a region. If not all voxels share the same material, -1 is returned.
    std::function<int(int, int, int, int)> get_mat = [&](int x, int y, int z, int width) -> int {
        if (width == 1) {
            if (x < 0 || x >= sz.x || y < 0 || y >= sz.y || z < 0 || z >= sz.z) {
                return 0;
            }
            return matrix.voxels[x * sz.y * sz.z + y * sz.z + z];
        }
        else {
            int mat = -2;
            int w = width / 2;
            for (int a = 0; a <= 1; ++a) {
                for (int b = 0; b <= 1; ++b) {
                    for (int c = 0; c <= 1; ++c) {
                        auto m = get_mat(x + a * w, y + b * w, z + c * w, w);
                        if (mat == -2) {
                            mat = m;
                        }
                        else if (mat != m) {
                            mat = -1;
                        }
                    }
                }
            }
            return mat;
        }
    };
    
    std::function<void(int, int, int, int, int)> build = [&](int index, int x, int y, int z, int width) {
        auto mat = get_mat(x, y, z, width);
        if (mat == -1) {
            // Subdivide
            int vi = octree.voxels.size();
            octree.voxels.resize(vi + 8);

            int w = width / 2;
            for (int a = 0; a <= 1; ++a) {
                for (int b = 0; b <= 1; ++b) {
                    for (int c = 0; c <= 1; ++c) {
                        build(vi + a * 4 + b * 2 + c, x + a * w, y + b * w, z + c * w, w);
                    }
                }
            }

            octree.voxels[index].material = octree.voxels[vi].material;
            octree.voxels[index].child = vi;
        }
        else {
            if (mat == -2) {
                mat = 0;
            }

            octree.voxels[index].material = mat;
            octree.voxels[index].child = 0;
        }
    };

    octree.voxels.resize(1);
    build(0, 0, 0, 0, radius);

    return std::move(octree);
}
