#pragma once

#include <cstddef>
#include <vector>
#include <memory>

#include "Layer.h"
#include "core/NonCopyable.h"
#include "runtime/ApplicationContext.h"

namespace conductor {
    class LayerStack : public NonCopyable {
    private:
        std::vector<std::unique_ptr<Layer>> m_layers;

        ApplicationContext& m_context;

    public:
        explicit LayerStack(ApplicationContext& context)
            : m_context(context) {};

        LayerStack(LayerStack&&) = delete;
        LayerStack& operator=(LayerStack&&) = delete;

        ~LayerStack();

        void push(std::unique_ptr<Layer> layer);
        void pop();

        void update(double delta_time);

        void clear();

        std::size_t size() const;

        bool empty() const;
    };
}