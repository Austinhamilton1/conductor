#include <utility>

#include "EventQueue.h"
#include "core/ConductorAssert.h"

namespace conductor {
    EventQueue::~EventQueue() {
        clear();
    }

    bool EventQueue::empty() const {
        return events.empty();
    }

    std::size_t EventQueue::size() const {
        return events.size();
    }

    void EventQueue::clear() {
        while(!events.empty()) {
            auto event = pop();

            if(event.destroy)
                event.destroy(event.payload);
        }
    }

    Event EventQueue::pop() {
        CONDUCTOR_ASSERT(!events.empty());

        Event event = std::move(events.front());
        events.pop();
        return event;
    }
}