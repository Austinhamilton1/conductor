#include "Subscription.h"
#include "EventDispatcher.h"

namespace conductor {
    Subscription::~Subscription() {
        release();
    }

    Subscription::Subscription(Subscription&& other) 
        : dispatcher(other.dispatcher),
        type(other.type),
        id(other.id) 
    {
        other.dispatcher = nullptr;
        other.type = 0;
        other.id = 0;
    }

    Subscription& Subscription::operator=(Subscription&& other) {
        if(this != &other) {
            release();

            dispatcher = other.dispatcher;
            type = other.type;
            id = other.id;

            other.dispatcher = nullptr;
            other.type = 0;
            other.id = 0;
        }

        return *this;
    }

    void Subscription::release() {
        if(dispatcher) {
            dispatcher->unsubscribe(type, id);
            dispatcher = nullptr;
        }
    }
}