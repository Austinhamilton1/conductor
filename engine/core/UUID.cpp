#include "UUID.h"

#include <random>

namespace conductor {
    UUID::UUID() {
        std::random_device rd;

        uuid = (uint64_t(rd()) << 32) | rd();
    }

    uint64_t UUID::value() const {
        return uuid;
    }
}