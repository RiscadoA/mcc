#pragma once

#include <string>
#include <glm/glm.hpp>

#include <mcc/result.hpp>
#include <mcc/config.hpp>
#include <mcc/gl/mesh.hpp>

namespace mcc::data {
    // Stores multiple meshes and their voxel data contained in a single .vox file.
    class Model final {
    public:
        Model(Model&& rhs);
        ~Model() = default;
        
        static Result<void, std::string> init(const Config& config); // Initializes the model loader and loads every model found
        static Result<const Model&, std::string> get(const std::string& name); // Gets a model
        
        Result<const gl::Mesh&, std::string> get_mesh(const std::string& name) const;
        
    private:
        static Result<void, std::string> iterate(const std::string& path, const std::string& name);
        static Result<void, std::string> parse_qb(std::ifstream& ifs, Model& model);

        Model() = default;

        static std::map<std::string, Model> models;
        std::map<std::string, gl::Mesh> meshes;
        std::map<std::string, std::vector<glm::u8vec4>> voxels;
    };
}