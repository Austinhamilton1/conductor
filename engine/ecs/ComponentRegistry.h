#pragma once

#include <cstddef>
#include <mutex>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <atomic>

#include "ComponentInfo.h"

namespace conductor {

    class ComponentRegistry {
    public:
        template<Component T>
        static ComponentTypeId register_component() {
            return register_type(
                std::type_index(typeid(T)),
                sizeof(T),
                alignof(T)
            );
        }

        template<Component T>
        static const ComponentInfo& info() {
            return info(
                register_component<T>()
            );
        }

        static const ComponentInfo& info(
            ComponentTypeId id
        );

        static void reserve(
            size_t capacity
        );

        static size_t size() noexcept;

        static void freeze();

        static bool frozen() noexcept;

    private:
        static ComponentTypeId register_type(
            std::type_index type,
            size_t size,
            size_t alignment
        );

        static std::vector<ComponentInfo> s_components;

        static std::unordered_map<
            std::type_index,
            ComponentTypeId
        > s_types;

        static std::mutex s_mutex;

        static std::atomic<bool> s_frozen;
    };

}