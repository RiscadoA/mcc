#pragma once

namespace mcc::map {
    class Generator {
    public:
        Generator(int seed, unsigned int chunk_size);

        inline int get_seed() const { return this->seed; }
        inline unsigned int get_chunk_size() const { return this->chunk_size; }

    private:
        const int seed;
        const unsigned int chunk_size;
    };
}
