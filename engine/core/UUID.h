#pragma once

#include <cstdint>

namespace conductor {
    class UUID {
    private:
        uint64_t uuid;

    public:
        UUID();

        uint64_t value() const;
    };
};