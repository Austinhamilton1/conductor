#pragma once

#include "runtime/ApplicationContext.h"

namespace conductor {
    class Layer {
    public:
        virtual ~Layer() = default;

        virtual void on_attach(ApplicationContext&) {}
        virtual void on_detach() {}

        virtual void on_update(double) {}
    };
}