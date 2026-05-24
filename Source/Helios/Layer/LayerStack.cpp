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

void helios::layer::LayerStack::onKeyEvent(helios::event::KeyEvent& event) {
    dispatchEvent(event, &ILayer::onKeyEvent);
}

void helios::layer::LayerStack::onMouseEvent(helios::event::MouseEvent& event) {
    dispatchEvent(event, &ILayer::onMouseEvent);
}

void helios::layer::LayerStack::onWindowEvent(helios::event::WindowEvent& event) {
    dispatchEvent(event, &ILayer::onWindowEvent);
}
