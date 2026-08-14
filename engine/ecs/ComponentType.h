#pragma once

#include <atomic>
#include <cstdint>
#include <type_traits>
#include <stdexcept>

namespace conductor {
    using ComponentTypeId = uint32_t;

    template<typename T>
    concept Component = 
        std::is_trivially_copyable_v<T> &&
        std::is_standard_layout_v<T>;
}