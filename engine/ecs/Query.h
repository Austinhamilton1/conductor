#pragma once

#include "ComponentType.h"

namespace conductor {
    template<Component T>
    struct Read {
        using component_type = T;

        static constexpr bool writable = false;
    };

    template<Component T>
    struct Write {
        using component_type = T;

        static constexpr bool writable = true;
    };
}