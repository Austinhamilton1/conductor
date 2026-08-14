#pragma once

#include <cstdint>

#include "ComponentType.h"

namespace conductor {
    struct ComponentInfo {
        ComponentTypeId id;
        uint32_t size;
        uint32_t alignment;

        bool operator==(const ComponentInfo& other) const {
            return id == other.id;
        }
    };
}