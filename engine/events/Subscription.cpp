#include "Subscription.h"
#include "EventDispatcher.h"

conductor::Subscription::~Subscription() {
    release();
}

conductor::Subscription::Subscription(Subscription&& other) 
    : dispatcher(other.dispatcher),
      type(other.type),
      id(other.id) 
{
    other.dispatcher = nullptr;
    other.type = 0;
    other.id = 0;
}

conductor::Subscription& conductor::Subscription::operator=(Subscription&& other) {
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

void conductor::Subscription::release() {
    if(dispatcher) {
        dispatcher->unsubscribe(type, id);
        dispatcher = nullptr;
    }
}