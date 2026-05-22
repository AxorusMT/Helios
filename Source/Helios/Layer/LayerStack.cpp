#include "Helios/Layer/LayerStack.h"

void helios::layer::LayerStack::popLayer() {
    if (layers.empty()) return;

    layers.back()->onDetach();
    layers.pop_back();
}

void helios::layer::LayerStack::update(float dt) {
    for (auto& layer : layers) {
        layer->update(dt);
    }
}

void helios::layer::LayerStack::draw() {
    for (auto& layer : layers) {
        layer->draw();
    }
}