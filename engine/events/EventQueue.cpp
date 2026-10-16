#include "EventQueue.h"

#include <optional>

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
        std::optional<Event> event;
        while(!(event = events.try_pop()).has_value()) {
            std::this_thread::yield();
        }

        return std::move(*event);
    }

    std::optional<Event> EventQueue::try_pop() {
        std::optional<Event> event;

        if((event = events.try_pop()).has_value()) {
            return std::move(*event);
        }

        return std::nullopt;
    }
}