#include "LayerStack.h"

#include <utility>

void conductor::LayerStack::push(std::unique_ptr<Layer> layer) {
    layer->on_attach();

    m_layers.push_back(
        std::move(layer)
    );
}

void conductor::LayerStack::pop() {
    if(!m_layers.empty()) {
        m_layers.back()->on_detach();
        m_layers.pop_back();
    }
}