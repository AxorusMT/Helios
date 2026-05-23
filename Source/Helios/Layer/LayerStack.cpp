#include "Helios/Layer/LayerStack.h"

helios::layer::LayerStack::~LayerStack() {
    clear();
}

void helios::layer::LayerStack::popLayer() {
    if (layers.empty()) return;

    layers.back()->onDetach();
    layers.back()->setLayerStack(nullptr);
    layers.pop_back();
}

void helios::layer::LayerStack::removeLayer(ILayer& layer) {
    for (auto it = layers.begin(); it != layers.end(); ++it) {
        if (it->get() != &layer) continue;

        (*it)->onDetach();
        (*it)->setLayerStack(nullptr);
        layers.erase(it);
        return;
    }
}

void helios::layer::LayerStack::clear() {
    while (!layers.empty()) {
        popLayer();
    }
}

void helios::layer::LayerStack::update(float dt) {
    const auto layer_count = layers.size();

    for (std::size_t i = 0; i < layer_count && i < layers.size(); ++i) {
        layers[i]->update(dt);
    }
}

void helios::layer::LayerStack::draw() {
    const auto layer_count = layers.size();

    for (std::size_t i = 0; i < layer_count && i < layers.size(); ++i) {
        layers[i]->draw();
    }
}

void helios::layer::LayerStack::onEvent(helios::event::IEvent& event) {
    std::vector<ILayer*> dispatch_layers;
    dispatch_layers.reserve(layers.size());

    for (auto it = layers.rbegin(); it != layers.rend(); ++it) {
        dispatch_layers.push_back(it->get());
    }

    for (ILayer* layer : dispatch_layers) {
        bool layer_is_attached = false;

        for (const auto& current_layer : layers) {
            if (current_layer.get() != layer) continue;

            layer_is_attached = true;
            break;
        }

        if (!layer_is_attached) continue;

        layer->onEvent(event);

        if (event.handled) return;
    }
}
