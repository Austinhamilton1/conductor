#include "UUID.h"

#include <random>

conductor::UUID::UUID() {
    std::random_device rd;

    uuid = (uint64_t(rd()) << 32) | rd();
}

uint64_t conductor::UUID::value() const {
    return uuid;
}