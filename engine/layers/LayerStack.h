#pragma once

#include <vector>
#include <memory>

#include "Layer.h"
#include "scheduler/Scheduler.h"

namespace conductor {
    class LayerStack {
    private:
        std::vector<std::unique_ptr<Layer>> m_layers;

    public:
        void push(std::unique_ptr<Layer> layer);

        void pop();
    };
}