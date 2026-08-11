#pragma once

#include <atomic>
#include <cstdint>
#include <type_traits>

namespace conductor {
    using ComponentTypeId = uint32_t;

    namespace detail {
        inline ComponentTypeId next_component_type_id() noexcept {
            static std::atomic<ComponentTypeId> next_id{0};
            return next_id.fetch_add(1, std::memory_order_relaxed);
        }

        template<typename T>
        ComponentTypeId component_type_id() noexcept {
            static const ComponentTypeId id = next_component_type_id();
            return id;
        }
    }

    template<typename T>
    concept Component = 
        std::is_trivially_copyable_v<T> &&
        std::is_standard_layout_v<T>;

    template<Component T>
    ComponentTypeId component_type_id() noexcept {
        return detail::component_type_id<T>();
    }
}