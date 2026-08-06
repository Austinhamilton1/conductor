#pragma once

#include "EventType.h"
#include "Subscription.h"
#include "Event.h"

#include <unordered_map>
#include <vector>
#include <functional>
#include <algorithm>
#include <type_traits>

namespace conductor {
    using Callback = std::function<void(void *)>;

    struct Listener {
        uint64_t id;
        Callback callback;
    };

    class EventDispatcher {
    private:
        uint64_t next_listener;
        std::unordered_map<EventType, std::vector<Listener>> registry;

    public:
        EventDispatcher() : next_listener(1) {};
        ~EventDispatcher() = default;

        template<typename T>
        Subscription subscribe(std::function<void(const T&)> callback);
        void unsubscribe(EventType, uint64_t id);       
        void dispatch(const Event& event);
    };
};

template<typename T>
conductor::Subscription conductor::EventDispatcher::subscribe(
    std::function<void(const T&)> callback
) {
    uint64_t id = next_listener++;

    Callback wrapper = [callback](void *ptr) {
        callback(*static_cast<T*>(ptr));
    };

    registry[event_type<std::decay_t<T>>()].push_back({
        id,
        wrapper
    });

    return Subscription(
        this,
        event_type<std::decay_t<T>>(),
        id
    );
}