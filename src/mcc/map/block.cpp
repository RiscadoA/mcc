#include <mcc/map/block.hpp>

using namespace mcc;
using namespace mcc::map;

Block::Type::Type(unsigned short id, const std::string& name, const glm::vec3& color, bool opaque) :
    id(id), name(name), color(color), opaque(opaque) {
    // Empty
}

Block::Registry::Registry() {
    this->block_types = {
        Block::Type(0, "air", glm::vec3(0.0f, 0.0f, 0.0f), false),
        Block::Type(1, "stone", glm::vec3(0.3f, 0.3f, 0.3f), true),
        Block::Type(2, "dirt", glm::vec3(0.8f, 0.3f, 0.3f), true),
        Block::Type(3, "grass", glm::vec3(0.0f, 0.8f, 0.3f), true),
    };
}

void Block::Registry::load(std::istream& in) {
    // TO DO
}

Result<const Block::Type&, std::string> Block::Registry::operator[](const char* name) const {
    for (auto& t : this->block_types) {
        if (t.get_name() == name) {
            return Result<const Block::Type&, std::string>::success(t);
        }
    }

    return Result<const Block::Type&, std::string>::error(
        "Block::Registry::operator[\"" + std::string(name) + "\"] failed:\n"
        "No block type with this name found");
}

const Block::Type& Block::Registry::operator[](unsigned short id) const {
    if (id >= this->block_types.size()) {
        std::cerr << "Block::Registry::operator[" << id << "] failed:\n";
        std::cerr << "There are only " << this->block_types.size() << " block types\n";
        std::abort();
    }

    return this->block_types[id];
}

Block::Block(Chunk* chunk, glm::u8vec3 pos) :
    chunk(chunk), pos(pos), prev(nullptr), next(nullptr) {
    // Empty
}

void Block::update(float dt) {
    // Empty
}

void Block::destroy() {
    // Empty
}
