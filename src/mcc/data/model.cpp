#include <mcc/data/model.hpp>
#include <mcc/data/qb_parser.hpp>

#include <filesystem>
#include <sstream>
#include <fstream>

using namespace mcc;
using namespace mcc::data;

Model::Model(Model&& rhs) : mesh(std::move(rhs.mesh)) {

}

const gl::Matrix& mcc::data::Model::get_matrix() const {
    return this->matrix;
}

const gl::Mesh& Model::get_mesh() const {
    return this->mesh;
}

Model::Loader::Loader(const Config& config) : config(config) {

}

Result<void, std::string> mcc::data::Model::Loader::load(int id) {
    auto& entry = this->get_entry(id);

    std::string path = this->config["data.folder"].unwrap().as_string() +
                       entry.arguments.substr(0, entry.arguments.find(' '));
    float scale = std::stof(entry.arguments.substr(entry.arguments.find(' ') + 1));
    
    if (this->models.size() <= id) {
        this->models.resize(id + 1, nullptr);
    }

    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        std::stringstream ss;
        ss << "mcc::data::Model::Loader::load() failed:" << std::endl;
        ss << "Couldn't open file \"" << path << "\"" << std::endl;
        return Result<void, std::string>::error(ss.str());
    }

    this->models[id] = new Model();
    auto result = parse_qb(ifs);
    if (result.is_error()) {
        std::stringstream ss;
        ss << "mcc::data::Model::Loader::load() failed:" << std::endl;
        ss << "Couldn't parse Qubicle Binary file:" << std::endl;
        ss << result.get_error();
        return Result<void, std::string>::error(ss.str());
    }
    this->models[id]->matrix = std::move(result.unwrap());

    this->models[id]->mesh.update(this->models[id]->matrix, scale);

    entry.ready = true;
    return Result<void, std::string>::success();
}

void mcc::data::Model::Loader::unload(int id) {
    delete this->models[id];
}
