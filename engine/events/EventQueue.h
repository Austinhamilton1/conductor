#pragma once

#include "Event.h"
#include "EventType.h"

#include <queue>
#include <utility>
#include <type_traits>

namespace conductor {
    class EventQueue {
    private:
        std::queue<Event> events;

    public:
        EventQueue() {};
        ~EventQueue();;

        bool empty() const;
        void clear();

        template<typename T>
        void push(T&& event);
        Event pop();

        template<typename T>
        static void destroy(void *ptr);

        template<typename F>
        void drain(F&& consumer);
    };
};

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

template<typename F>
void conductor::EventQueue::drain(F&& consumer) {
    while(!events.empty()) {
        Event event = pop();

        consumer(event);

        if(event.destroy)
            event.destroy(event.payload);
    }
}