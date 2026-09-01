#pragma once

#include <concepts>

#include "Query.h"

namespace conductor {
    template<typename T>
    concept System = 
        requires {
            typename T::Query;

            requires QueryType<
                typename T::Query
            >;
        };
}