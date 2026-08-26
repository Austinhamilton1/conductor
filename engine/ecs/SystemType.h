#pragma once

#include <concepts>

#include "Query.h"
#include "SystemContext.h"

namespace conductor {
    template<typename T>
    concept System =
        requires {
            typename T::Query;

            requires QueryType<typename T::Query>;
        }
        &&
        requires(T& system, SystemContext<typename T::Query>& ctx) {
            {system.update(ctx)} -> std::same_as<void>;
        };
}