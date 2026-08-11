#include "Random.h"
#include "Assert.h"

namespace conductor {
    void Random::seed(uint64_t s) {
        this->s = s;
        generator = std::mt19937_64(s);
    }

    uint64_t Random::get_seed() {
        return s;
    }

    int Random::next_int(int min_value, int max_value) {
        std::uniform_int_distribution<int> dis(min_value, max_value);
        return dis(generator);
    }

    float Random::next_float() {
        std::uniform_real_distribution<float> dis(0.0f, 1.0f);
        return dis(generator);
    }

    double Random::next_double() {
        std::uniform_real_distribution<double> dis(0.0, 1.0);
        return dis(generator);
    }

    bool Random::chance(float p) {
        CONDUCTOR_ASSERT(p <= 1.0f && p >= 0.0f);
        
        std::uniform_real_distribution<float> dis(0.0f, 1.0f);
        float test = dis(generator);
        return test < p;
    }
}