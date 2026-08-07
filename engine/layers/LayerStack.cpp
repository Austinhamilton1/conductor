#include "LayerStack.h"

#include <utility>

conductor::LayerStack::~LayerStack() {
    clear();
}

void conductor::LayerStack::push(std::unique_ptr<Layer> layer) {
    if(!layer)
        return;

    m_layers.push_back(std::move(layer));

    m_layers.back()->on_attach(m_context);
}

void conductor::LayerStack::pop() {
    if(m_layers.empty())
        return;

    m_layers.back()->on_detach();
    m_layers.pop_back();
}

void conductor::LayerStack::update(double delta_time) {
    for(auto& layer : m_layers) {
        layer->on_update(delta_time);
    }
}

void conductor::LayerStack::clear() {
    while(!m_layers.empty()) {
        pop();
    }
}

std::size_t conductor::LayerStack::size() const {
    return m_layers.size();
}

bool conductor::LayerStack::empty() const {
    return m_layers.empty();
}