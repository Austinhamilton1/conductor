#include "EventBus.h"

void conductor::EventBus::process() {
    queue.drain([this](const Event& event) {
        dispatcher.dispatch(event);
    });
}