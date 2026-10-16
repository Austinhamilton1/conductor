#include "EventBus.h"

#include <cstddef>
#include <optional>

namespace conductor {
    void EventBus::process() {
        // We get the current size of the queue so
        // that potential events published during another event
        // are not processed recursively.
        const std::size_t count = queue.size();

        // When we get the size, it is approximate; however,
        // because this is called prior to consuming, it acts as
        // a lower bound, so it is ok to use.
        for(std::size_t i = 0; i < count; i++) {
            if(auto event = queue.try_pop()) {
                dispatcher.dispatch(*event);

                if(event->destroy)
                    event->destroy(event->payload);
            }
        }
    }
}