#pragma once

#include "scheduler/Scheduler.h"

namespace conductor {
    class Layer {
    public:
        virtual ~Layer() = default;

        virtual void on_attach() {}
        virtual void on_detach() {}
    };
};