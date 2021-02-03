#pragma once

#include <mcc/result.hpp>

#include <atomic>
#include <vector>
#include <string>

namespace mcc::data {
    class Loader {
    public:
        Loader() = default;
        Loader(const Loader&) = delete;
        Loader(Loader&&) = default;
        virtual ~Loader() = default;

        Result<void, std::string> add_entry(const std::string& id, bool dynamic, const std::string& arguments);
        void inc_ref(int id);
        void dec_ref(int id);
        bool is_ready(int id);
        void wait(int id);

        int get_asset(const std::string& id);

    protected:
        struct Entry {
            std::string id;
            std::string arguments;
            std::atomic<unsigned int> ref_count;
            std::atomic<bool> ready;
            bool dynamic;

            Entry() = default;
            Entry(const Entry& rhs);
        };

        Entry& get_entry(int id);

        virtual Result<void, std::string> load(int id) = 0;
        virtual void unload(int id) = 0;

    private:
        std::vector<Entry> entries;
    };
}