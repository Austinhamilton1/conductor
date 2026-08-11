#pragma once

#include <cstdint>

namespace conductor {
    class UUID {
    public:
        UUID();
        uint64_t value() const;

    private:
        uint64_t uuid;
    };
}