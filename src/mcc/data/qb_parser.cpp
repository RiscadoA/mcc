#include <mcc/data/qb_parser.hpp>
#include <mcc/memory/endianness.hpp>

#include <fstream>

using namespace mcc;
using namespace mcc::data;

Result<gl::Matrix, std::string> mcc::data::parse_qb(std::ifstream& ifs) {
    uint8_t version[4];
    uint32_t color_format, z_axis_orientation, compressed, visibility_mask_encoded, num_matrices;

    // Parse file header
    ifs.read((char*)version, 4);
    if (version[0] != 1 || version[1] != 1 || version[2] != 0 || version[3] != 0) {
        return Result<gl::Matrix, std::string>::error(
            "Unsupported QB file format version"
        );
    }
    ifs.read((char*)&color_format, 4);
    color_format = memory::from_big_endian(color_format);
    ifs.read((char*)&z_axis_orientation, 4);
    z_axis_orientation = memory::from_big_endian(z_axis_orientation);
    ifs.read((char*)&compressed, 4);
    compressed = memory::from_big_endian(compressed);
    ifs.read((char*)&visibility_mask_encoded, 4);
    visibility_mask_encoded = memory::from_big_endian(visibility_mask_encoded);
    ifs.read((char*)&num_matrices, 4);
    num_matrices = memory::from_big_endian(num_matrices);

    if (num_matrices != 1) {
        return Result<gl::Matrix, std::string>::error(
            "Unsupported QB file, each file must have exactly one matrix"
        );
    }

    // Read matrix name
    uint8_t name_length;
    ifs.read((char*)&name_length, 1);
    auto name = std::string(name_length, ' ');
    ifs.read(&name[0], name_length);

    // Read matrix size and position
    uint32_t size_x, size_y, size_z, pos_x, pos_y, pos_z;
    ifs.read((char*)&size_x, 4);
    size_x = memory::from_big_endian(size_x);
    ifs.read((char*)&size_y, 4);
    size_y = memory::from_big_endian(size_y);
    ifs.read((char*)&size_z, 4);
    size_z = memory::from_big_endian(size_z);
    ifs.read((char*)&pos_x, 4);
    pos_x = memory::from_big_endian(pos_x);
    ifs.read((char*)&pos_y, 4);
    pos_y = memory::from_big_endian(pos_y);
    ifs.read((char*)&pos_z, 4);
    pos_z = memory::from_big_endian(pos_z);

    // Read matrix data
    gl::Matrix matrix;
    matrix.size = { size_x, size_y, size_z };
    int mat_count = 1;
    matrix.voxels.resize(size_x * size_y * size_z, 0);
    if (compressed == 0) { // If uncompressed
        uint8_t color[4];
        for (auto z = 0u; z < size_z; ++z) {
            for (auto y = 0u; y < size_y; ++y) {
                for (auto x = 0u; x < size_x; ++x) {
                    ifs.read((char*)color, 4);
                    if (color[3] == 0) {
                        continue;
                    }

                    if (color_format) {
                        std::swap(color[0], color[2]);
                    }

                    int mat_id = 1;

                    for (; mat_id < mat_count; ++mat_id) {
                        if (matrix.palette[mat_id].color.r == color[0] &&
                            matrix.palette[mat_id].color.g == color[1] &&
                            matrix.palette[mat_id].color.b == color[2]) {
                            break;
                        }
                    }

                    if (mat_id == mat_count) {
                        ++mat_count;
                        if (mat_count > 256) {
                            return Result<gl::Matrix, std::string>::error(
                                "Unsupported QB file, too many voxel colors (palette is full)"
                            );
                        }

                        matrix.palette[mat_id].color.r = color[0];
                        matrix.palette[mat_id].color.g = color[1];
                        matrix.palette[mat_id].color.b = color[2];
                        matrix.palette[mat_id].color.a = 255;
                    }

                    matrix.voxels[x * size_y * size_z + y * size_z + z] = mat_id;
                }
            }
        }
    }
    else { // If compressed
        // TO DO
    }

    return Result<gl::Matrix, std::string>::success(std::move(matrix));
}
