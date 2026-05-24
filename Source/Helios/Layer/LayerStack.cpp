#include "Helios/Layer/LayerStack.h"

helios::layer::LayerStack::~LayerStack() {
    clear();
}

helios::layer::ILayer& helios::layer::LayerStack::pushLayer(std::unique_ptr<ILayer> layer) {
    assert(layer != nullptr);

    ILayer& ref = *layer;

    layer->setLayerHandle(LayerHandle(next_layer_id++));
    layer->setLayerContext(this, &world);
    layer->onAttach();
    layers.push_back(std::move(layer));

    return ref;
}

void helios::layer::LayerStack::popLayer() {
    if (layers.empty()) return;

    layers.back()->onDetach();
    layers.back()->setLayerContext(nullptr, nullptr);
    layers.back()->setLayerHandle(LayerHandle());
    layers.pop_back();
}

void helios::layer::LayerStack::removeLayer(ILayer& layer) {
    for (auto it = layers.begin(); it != layers.end(); ++it) {
        if (it->get() != &layer) continue;

        (*it)->onDetach();
        (*it)->setLayerContext(nullptr, nullptr);
        (*it)->setLayerHandle(LayerHandle());
        layers.erase(it);
        return;
    }
}

bool helios::layer::LayerStack::removeLayer(LayerHandle handle) {
    if (!handle.isValid()) return false;

    for (auto it = layers.begin(); it != layers.end(); ++it) {
        if ((*it)->getLayerHandle() != handle) continue;

        (*it)->onDetach();
        (*it)->setLayerContext(nullptr, nullptr);
        (*it)->setLayerHandle(LayerHandle());
        layers.erase(it);
        return true;
    }

    return false;
}

bool helios::layer::LayerStack::containsLayer(LayerHandle handle) const {
    return findLayer(handle) != nullptr;
}

void helios::layer::LayerStack::clear() {
    while (!layers.empty()) {
        popLayer();
    }
}

void helios::layer::LayerStack::update(float dt) {
    std::vector<LayerHandle> update_layers;
    update_layers.reserve(layers.size());

    for (const auto& layer : layers) {
        update_layers.push_back(layer->getLayerHandle());
    }

    for (LayerHandle handle : update_layers) {
        ILayer* layer = findLayer(handle);
        if (layer == nullptr) continue;

        layer->update(dt);
    }
}

void helios::layer::LayerStack::draw() {
    std::vector<LayerHandle> draw_layers;
    draw_layers.reserve(layers.size());

    for (const auto& layer : layers) {
        draw_layers.push_back(layer->getLayerHandle());
    }

    for (LayerHandle handle : draw_layers) {
        ILayer* layer = findLayer(handle);
        if (layer == nullptr) continue;

        layer->draw();
    }
}

helios::layer::ILayer* helios::layer::LayerStack::findLayer(LayerHandle handle) const {
    if (!handle.isValid()) return nullptr;

    for (const auto& layer : layers) {
        if (layer->getLayerHandle() == handle) {
            return layer.get();
        }
    }

    return nullptr;
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
