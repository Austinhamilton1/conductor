#include <algorithm>

#include "EventDispatcher.h"

namespace conductor {
    void EventDispatcher::unsubscribe(EventType type, uint64_t id) {
        auto it = registry.find(type);

        if(it == registry.end())
            return;

        auto& listeners = it->second;

        listeners.erase(
            std::remove_if(
                listeners.begin(),
                listeners.end(),
                [id](const Listener& l) {
                    return l.id == id;
                }
            ),
            listeners.end()
        );
    }

    void EventDispatcher::dispatch(const Event& event) {
        auto it = registry.find(event.type);

        if(it == registry.end())
            return;

        for(auto& listener : it->second) {
            listener.callback(event.payload);
        }
    }
}