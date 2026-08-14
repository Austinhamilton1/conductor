#include "ComponentRegistry.h"

#include <stdexcept>
#include <limits>

namespace conductor {
    std::vector<ComponentInfo> ComponentRegistry::s_components;
    std::mutex ComponentRegistry::s_mutex;

    std::atomic<bool> ComponentRegistry::s_frozen{false};

    ComponentTypeId ComponentRegistry::allocate(
        size_t size,
        size_t alignment
    ) {
        std::scoped_lock lock(s_mutex);

        if(s_frozen.load(std::memory_order_acquire)) {
            throw std::logic_error(
                "Cannot register a component after "
                "ComponentRegistry has been frozen"
            );
        }

        if(size >
            static_cast<size_t>(
                std::numeric_limits<uint32_t>::max()
            )) {
                throw std::length_error(
                    "Component size exceeds uint32_t range"
                );
            }

        if(alignment >
            static_cast<size_t>(
                std::numeric_limits<uint32_t>::max()
            )) {
                throw std::length_error(
                    "Component alignment exceeds uint32_t range"
                );
            }

        const auto id =
            static_cast<ComponentTypeId>(
                s_components.size()
            );

        s_components.push_back(
            ComponentInfo{
                id,
                static_cast<uint32_t>(size),
                static_cast<uint32_t>(alignment)
            }
        );

        return id;
    }

    const ComponentInfo& ComponentRegistry::info(
        ComponentTypeId id
    ) {
        if(!s_frozen.load(std::memory_order_acquire)) {
            std::scoped_lock lock(s_mutex);

            if(id >= s_components.size()) {
                throw std::out_of_range(
                    "Invalid component type ID"
                );
            }

            return s_components[id];
        }

        if(id >= s_components.size()) {
            throw std::out_of_range(
                "Invalid component type ID"
            );
        }

        return s_components[id];
    }

    void ComponentRegistry::reserve(size_t capacity) {
        std::scoped_lock lock(s_mutex);

        if(s_frozen) {
            throw std::logic_error(
                "Cannot reserve Component Registry after freeze"
            );
        }

        s_components.reserve(capacity);
    }

    size_t ComponentRegistry::size() noexcept {
        if(!s_frozen.load(std::memory_order_acquire)) {
            std::scoped_lock lock(s_mutex);

            return s_components.size();
        }

        return s_components.size();
    }

    void ComponentRegistry::freeze() {
        std::scoped_lock lock(s_mutex);

        s_frozen.store(
            true,
            std::memory_order_release
        );
    }

    bool ComponentRegistry::frozen() noexcept {
        return s_frozen.load(std::memory_order_acquire);
    }
}