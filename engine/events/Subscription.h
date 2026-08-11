#pragma once

#include "core/NonCopyable.h"
#include "EventType.h"

namespace conductor {
    class EventDispatcher; 

    class Subscription : public NonCopyable {
    public:
        Subscription() = default;

        Subscription(
            EventDispatcher *dispatcher,
            EventType type,
            uint64_t id
        )
            : dispatcher(dispatcher),
              type(type),
              id(id) {};

        ~Subscription();

        Subscription(Subscription&&);

        Subscription& operator=(Subscription&&);

    private:
        EventDispatcher *dispatcher = nullptr;
        EventType type = 0;
        uint64_t id = 0;
        
        void release();
    };
}