#include "EventQueue.h"

#include <cassert>
#include <optional>
#include <utility>

namespace conductor {
    EventQueue::~EventQueue() {
        clear();
    }

    bool EventQueue::empty() const {
        return events.empty_approx();
    }

    std::size_t EventQueue::size() const {
        return events.size_approx();
    }

    void EventQueue::clear() {
        std::optional<Event> event;
        while((event = events.try_pop()).has_value()) {
            if(event->destroy)
                event->destroy(event->payload);
        }
    }

    Event EventQueue::pop() {
        auto event = events.try_pop();
        assert(event.has_value());

        return std::move(*event);
    }
}