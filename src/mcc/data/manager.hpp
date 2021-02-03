#pragma once

#include <string>
#include <map>

#include <mcc/result.hpp>
#include <mcc/config.hpp>
#include <mcc/data/handle.hpp>
#include <mcc/data/loader.hpp>

namespace mcc::data {
    /**
     * Manages asset loading.
     */
    class Manager final {
    public:
        Manager(const Config& config, const std::map<std::string, Loader*>& loaders);
        ~Manager() = default;
        
        Result<Handle<void>, std::string> get(const std::string& id);

        template <typename T>
        inline Result<Handle<T>, std::string> get(const std::string& id);

    private:
        std::map<std::string, Loader*> loaders_by_asset;
        std::map<std::string, Loader*> loaders_by_type;
    };

    template<typename T>
    inline Result<Handle<T>, std::string> Manager::get(const std::string& id) {
        auto handle = this->get(id);
        if (handle.is_error()) {
            return Result<Handle<T>, std::string>::error(handle.get_error());
        }
        else {
            return Result<Handle<T>, std::string>::success(Handle<T>(handle.unwrap()));
        }
    }
}