#include <utility>

#include "EventQueue.h"
#include "core/Assert.h"

conductor::EventQueue::~EventQueue() {
    clear();
}

bool conductor::EventQueue::empty() const {
    return events.empty();
}

void conductor::EventQueue::clear() {
    while(!events.empty()) {
        auto event = pop();

        if(event.destroy)
            event.destroy(event.payload);
    }
}

conductor::Event conductor::EventQueue::pop() {
    CONDUCTOR_ASSERT(!events.empty());

    Event event = std::move(events.front());
    events.pop();
    return event;
}