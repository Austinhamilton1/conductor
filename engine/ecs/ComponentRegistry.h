#pragma once

#include <cstddef>
#include <mutex>
#include <vector>
#include <atomic>

#include "ComponentInfo.h"

namespace conductor {
    class ComponentRegistry {
    public:
        template<Component T>
        static ComponentTypeId register_component() {
            static const ComponentTypeId id =
                allocate(sizeof(T), alignof(T));

            return id;
        }

        template<Component T>
        static const ComponentInfo& info() {
            return info(register_component<T>());
        }

        static const ComponentInfo& info(
            ComponentTypeId id
        );

        static void reserve(size_t capacity);

        static size_t size() noexcept;

        static void freeze();

        static bool frozen() noexcept;

    private:
        static ComponentTypeId allocate(
            size_t size,
            size_t alignment
        );

        static std::vector<ComponentInfo> s_components;
        static std::mutex s_mutex;
        static std::atomic<bool> s_frozen;
    };
}