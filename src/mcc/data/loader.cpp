#include <mcc/data/loader.hpp>

using namespace mcc;
using namespace mcc::data;

void Loader::inc_ref(int id) {
    if (this->entries[id].ref_count == 0 &&
        this->entries[id].dynamic) {
        this->load(id).unwrap();
    }
    this->entries[id].ref_count += 1;
}

void Loader::dec_ref(int id) {
    this->entries[id].ref_count -= 1;

    if (this->entries[id].ref_count == 0 &&
        this->entries[id].dynamic) {
        this->wait(id);
        this->unload(id);
        this->entries[id].ready = false;
    }
}

Result<void, std::string> Loader::add_entry(const std::string& id, bool dynamic, const std::string& arguments) {
    this->entries.emplace_back();
    this->entries.back().id = id;
    this->entries.back().dynamic = dynamic;
    this->entries.back().arguments = arguments;
    this->entries.back().ready = false;
    this->entries.back().ref_count = 0;
    if (!dynamic) {
        return this->load(this->entries.size() - 1);
    }
    else {
        return Result<void, std::string>::success();
    }
}

bool mcc::data::Loader::is_ready(int id) {
    return this->entries[id].ready;
}

void mcc::data::Loader::wait(int id) {
    while (!this->entries[id].ready);
}

int mcc::data::Loader::get_asset(const std::string& id) {
    for (int i = 0; i < this->entries.size(); ++i) {
        if (this->entries[i].id == id) {
            return i;
        }
    }

    return -1;
}

Loader::Entry& Loader::get_entry(int id) {
    return this->entries[id];
}

mcc::data::Loader::Entry::Entry(const Entry& rhs) {
    this->id = rhs.id;
    this->arguments = rhs.arguments;
    this->ready.store(rhs.ready.load());
    this->ref_count.store(rhs.ref_count.load());
    this->dynamic = rhs.dynamic;
}
