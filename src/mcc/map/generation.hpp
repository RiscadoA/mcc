#pragma once

#include <cstdint>

namespace mcc::map {
    // Holds map generation settings
    class Generation {
    public:
        Generation(uint64_t seed);
        ~Generation() = default;

        inline uint64_t get_seed() const { return this->seed; }
        
    private:
        uint64_t seed;
    };
}