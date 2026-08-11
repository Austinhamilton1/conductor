#include "LayerStack.h"

#include <utility>

namespace conductor {
    LayerStack::~LayerStack() {
        clear();
    }

    void LayerStack::push(std::unique_ptr<Layer> layer) {
        if(!layer)
            return;

        m_layers.push_back(std::move(layer));

        m_layers.back()->on_attach(m_context);
    }

    void LayerStack::pop() {
        if(m_layers.empty())
            return;

        m_layers.back()->on_detach();
        m_layers.pop_back();
    }

    void LayerStack::update(double delta_time) {
        for(auto& layer : m_layers) {
            layer->on_update(delta_time);
        }
    }

    void LayerStack::clear() {
        while(!m_layers.empty()) {
            pop();
        }
    }

    size_t LayerStack::size() const {
        return m_layers.size();
    }

    bool LayerStack::empty() const {
        return m_layers.empty();
    }
}