#pragma once

#include "EventQueue.h"
#include "EventDispatcher.h"

#include <utility>
#include <functional>

namespace conductor {
    class EventBus {
    public:
        EventBus(std::size_t capacity = 4096) : queue(capacity) {}
        
        template<typename T>
        void publish(T&&  event);

        template<typename T>
        Subscription subscribe(std::function<void(const T&)> callback);

        void process();

    private:
        EventQueue queue;
        EventDispatcher dispatcher;
    };
}

template<typename T>
void conductor::EventBus::publish(T&& event) {
    static_assert(
        !std::is_pointer_v<std::decay_t<T>>,
        "Events should be value types"
    );
    
    queue.push(std::forward<T>(event));
}

template<typename T>
conductor::Subscription conductor::EventBus::subscribe(
    std::function<void(const T&)> callback
) {
    return dispatcher.subscribe<T>(
        std::move(callback)
    );
}