#pragma once

#include <vector>

#include <mcc/result.hpp>
#include <mcc/data/loader.hpp>
#include <mcc/gl/mesh.hpp>
#include <mcc/config.hpp>

namespace mcc::data {
    // Stores multiple meshes and their voxel data contained in a single .vox file.
    class Model final {
    public:
        class Loader final : public data::Loader {
        public:
            Loader(const Config& config);

            inline const Model* get(int id) { return this->models[id]; }
            virtual Result<void, std::string> load(int id) final;
            virtual void unload(int id) final;

        private:
            const Config& config;

            std::vector<Model*> models;
        };

        Model(Model&& rhs);
        ~Model() = default;
               
        const gl::Matrix& get_matrix() const;
        const gl::Mesh& get_mesh() const;
        
    private:

        Model() = default;

        gl::Matrix matrix;
        gl::Mesh mesh;
    };
}