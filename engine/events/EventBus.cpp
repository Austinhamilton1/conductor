#include <cstddef>

#include "EventBus.h"

namespace conductor {
    void conductor::EventBus::process() {
        const std::size_t count = queue.size();

        for(std::size_t i = 0; i < count; i++) {
            Event event = queue.pop();

            dispatcher.dispatch(event);

            if(event.destroy)
                event.destroy(event.payload);
        }
    }
}