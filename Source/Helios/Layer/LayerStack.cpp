#include "Helios/Layer/LayerStack.h"

helios::layer::LayerStack::~LayerStack() {
    clear();
}

helios::layer::ILayer& helios::layer::LayerStack::pushLayer(std::unique_ptr<ILayer> layer) {
    assert(layer != nullptr);

    ILayer& ref = *layer;

    layer->setLayerContext(this, &world);
    layer->onAttach();
    layers.push_back(std::move(layer));

    return ref;
}

void helios::layer::LayerStack::popLayer() {
    if (layers.empty()) return;

    layers.back()->onDetach();
    layers.back()->setLayerContext(nullptr, nullptr);
    layers.pop_back();
}

void helios::layer::LayerStack::removeLayer(ILayer& layer) {
    for (auto it = layers.begin(); it != layers.end(); ++it) {
        if (it->get() != &layer) continue;

        (*it)->onDetach();
        (*it)->setLayerContext(nullptr, nullptr);
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

void helios::layer::LayerStack::onKeyHeldEvent(helios::event::KeyHeldEvent& event) {
    dispatchEvent(event, &ILayer::onKeyHeldEvent);
}

void helios::layer::LayerStack::onKeyPressedEvent(helios::event::KeyPressedEvent& event) {
    dispatchEvent(event, &ILayer::onKeyPressedEvent);
}

void helios::layer::LayerStack::onKeyReleasedEvent(helios::event::KeyReleasedEvent& event) {
    dispatchEvent(event, &ILayer::onKeyReleasedEvent);
}

void helios::layer::LayerStack::onMouseButtonPressedEvent(helios::event::MouseButtonPressedEvent& event) {
    dispatchEvent(event, &ILayer::onMouseButtonPressedEvent);
}

void helios::layer::LayerStack::onMouseButtonReleasedEvent(helios::event::MouseButtonReleasedEvent& event) {
    dispatchEvent(event, &ILayer::onMouseButtonReleasedEvent);
}

void helios::layer::LayerStack::onMouseMovedEvent(helios::event::MouseMovedEvent& event) {
    dispatchEvent(event, &ILayer::onMouseMovedEvent);
}

void helios::layer::LayerStack::onMouseScrolledEvent(helios::event::MouseScrolledEvent& event) {
    dispatchEvent(event, &ILayer::onMouseScrolledEvent);
}

void helios::layer::LayerStack::onWindowClosedEvent(helios::event::WindowClosedEvent& event) {
    dispatchEvent(event, &ILayer::onWindowClosedEvent);
}

void helios::layer::LayerStack::onWindowResizedEvent(helios::event::WindowResizedEvent& event) {
    dispatchEvent(event, &ILayer::onWindowResizedEvent);
}
