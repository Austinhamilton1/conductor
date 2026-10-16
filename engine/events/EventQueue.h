#pragma once

#include "Event.h"
#include "EventType.h"
#include "core/MPMC.h"

#include <cstddef>
#include <optional>
#include <thread>
#include <type_traits>
#include <utility>

namespace conductor {
    class EventQueue {
    public:
        EventQueue(std::size_t capacity = 4096)
            : events(capacity) {};
        ~EventQueue();

        bool empty() const;
        void clear();
        std::size_t size() const;

        template<typename T>
        void push(T&& event);
        Event pop();

        template<typename T>
        bool try_push(T&& event);
        std::optional<Event> try_pop();

        template<typename T>
        static void destroy(void *ptr);

    private:
        MPMC<Event> events;
    };

    template<typename T>
    void EventQueue::push(T&& event) {
        using U = std::decay_t<T>;

        Event e;

        e.type = event_type<U>();
        e.payload = new U(std::forward<T>(event));
        e.destroy = &EventQueue::destroy<U>;

        while(!events.try_push(e)) {
            std::this_thread::yield();
        }
    }

    template<typename T>
    bool EventQueue::try_push(T&& event) {
        using U = std::decay_t<T>;

        Event e;

        e.type = event_type<U>();
        e.payload = new U(std::forward<T>(event));
        e.destroy = &EventQueue::destroy<U>;

        return events.try_push(e);
    }

    template<typename T>
    void EventQueue::destroy(void *ptr) {
        delete static_cast<T*>(ptr);
    }
}