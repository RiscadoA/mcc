#include <mcc/data/model.hpp>
#include <mcc/memory/endianness.hpp>

#include <filesystem>
#include <fstream>

using namespace mcc;
using namespace mcc::data;

std::map<std::string, Model> Model::models = std::map<std::string, Model>();

Result<void, std::string> Model::init(const Config& config) {
    auto path = config["data.folder"].unwrap().as_string() + "model/";
    auto result = Model::iterate(path, "");
    if (result.is_error()) {
        return Result<void, std::string>::error(
            "Model::init() failed:\n" +
            result.get_error()
        );
    } else {
        return result;
    }
}

Result<const Model&, std::string> Model::get(const std::string& name) {
    auto it = Model::models.find(name);
    if (it == Model::models.end()) {
        return Result<const Model&, std::string>::error("Model::get(\"" + name + "\") failed:\nNo model found with this name");
    }

    return Result<const Model&, std::string>::success(it->second);
}

Result<void, std::string> Model::iterate(const std::string& path, const std::string& name) {
    for (auto f : std::filesystem::directory_iterator(path)) {
        if (f.is_directory()) {
            auto result = Model::iterate(f.path().string(), name + f.path().filename().string() + ".");
            if (result.is_error()) {
                return result;
            }
        } else if (f.is_regular_file()) {
            auto path = f.path().string();
            auto model = Model();

            if (!f.path().has_extension() || f.path().extension().string() != ".qb") {
                return Result<void, std::string>::error(
                    "Model::iterate() failed:\n"
                    "Unknown file format \"" + f.path().string() + "\""    
                );
            }

            std::ifstream ifs(path, std::ifstream::binary);
            if (!ifs) {
                return Result<void, std::string>::error(
                    "Model::iterate() failed:\n"
                    "std::ifstream() failed on \"" + f.path().string() + "\""    
                );
            }

            auto result = Model::parse_qb(ifs, model);
            if (result.is_error()) {
                return Result<void, std::string>::error(
                    "Model::iterate() failed:\n"
                    "Model::parse_qb() failed on \"" + f.path().string() + "\"\n" +
                    result.get_error()
                );
            }

            auto model_name = name + f.path().filename().string();
            model_name = model_name.substr(0, model_name.size() - 3); // Remove extension
            Model::models.emplace(std::make_pair(model_name, std::move(model)));
        }
    }

    return Result<void, std::string>::success();
}

Result<void, std::string> Model::parse_qb(std::ifstream& ifs, Model& model) {
    uint8_t version[4];
    uint32_t color_format, z_axis_orientation, compressed, visibility_mask_encoded, num_matrices;
    
    // Parse file header
    ifs.read((char*)version, 4);
    if (version[0] != 1 || version[1] != 1 || version[2] != 0 || version[3] != 0) {
        return Result<void, std::string>::error(
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
    
    for (auto i = 0u; i < num_matrices; ++i) {
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
        auto& voxels = model.voxels[name];
        voxels.resize(size_x * size_y * size_z, glm::u8vec4(0, 0, 0, 0));
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
                            voxels[x * size_y * size_z + y * size_z + z] = glm::u8vec4(color[2], color[1], color[0], 255);
                        } else {
                            voxels[x * size_y * size_z + y * size_z + z] = glm::u8vec4(color[0], color[1], color[2], 255);
                        }
                    }
                }
            }
        } else { // If compressed
            // TO DO
        }

        model.meshes[name].build_buffers(&voxels[0], glm::uvec3(size_x, size_y, size_z), 1.0f, true);
        model.meshes[name].build_va();
    }


    return Result<void, std::string>::success();
}

Model::Model(Model&& rhs) {
    this->meshes = std::move(rhs.meshes);
}

Result<const gl::Mesh&, std::string> Model::get_mesh(const std::string& name) const {
    auto it = this->meshes.find(name);
    if (it == this->meshes.end()) {
        return Result<const gl::Mesh&, std::string>::error("Model::get_mesh() failed:\nNo mesh with name '" + name + "' found");
    } else {
        return Result<const gl::Mesh&, std::string>::success(it->second);
    }
}
