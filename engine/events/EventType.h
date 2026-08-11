#pragma once

#include <cstdint>
#include <atomic>

namespace conductor {
    using EventType = uint32_t;

    namespace detail {
        inline std::atomic<EventType> next_event_type = 0;
    }

    template<typename T>
    EventType event_type();
}

template<typename T>
conductor::EventType conductor::event_type() {
    static const EventType type = detail::next_event_type++;
    return type;
}