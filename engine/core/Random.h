#pragma once

#include <cstdint>
#include <random>

namespace conductor {
    class Random {
    private:
        uint64_t s;
        std::mt19937_64 generator;

    public:
        explicit Random(uint64_t s) 
            : s(s), 
              generator(std::mt19937_64(s)) {};

        void seed(uint64_t s);
        uint64_t get_seed();

        int next_int(int min_value, int max_value);
        float next_float();
        double next_double();
        bool chance(float p);
    };
};