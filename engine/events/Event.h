#pragma once

#include "EventType.h"

#include <memory>

namespace conductor {
    struct Event {
        EventType type = 0;
        void *payload = nullptr;
        void (*destroy)(void *) = nullptr;
    };
}