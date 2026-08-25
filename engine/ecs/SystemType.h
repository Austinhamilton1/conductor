#pragma once

#include <concepts>
#include <string>

#include "SystemContext.h"

namespace conductor {
    template<typename T>
    concept System = requires(T system, SystemContext<typename T::Query>& ctx) {
        // Query is a defined subtype for compile-time guarantees
        typename T::Query;

        { system.update(ctx) } -> std::same_as<void>;
    };
}