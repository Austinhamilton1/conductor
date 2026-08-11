#pragma once

#include "Event.h"
#include "EventType.h"

#include <queue>
#include <utility>
#include <type_traits>
#include <cstddef>

namespace conductor {
    class EventQueue {
    public:
        EventQueue() {};
        ~EventQueue();

        bool empty() const;
        void clear();
        std::size_t size() const;

        template<typename T>
        void push(T&& event);
        Event pop();

        template<typename T>
        static void destroy(void *ptr);

    private:
        std::queue<Event> events;
    };
}

template<typename T>
void conductor::EventQueue::push(T&& event) {
    using U = std::decay_t<T>;

    Event e;

    e.type = event_type<U>();
    e.payload = new U(std::forward<T>(event));
    e.destroy = &EventQueue::destroy<U>;

    events.push(e);
}

template<typename T>
void conductor::EventQueue::destroy(void *ptr) {
    delete static_cast<T*>(ptr);
}