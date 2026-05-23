#pragma once

#include "ILayer.h"

#include <Helios/Core.h>

namespace helios::layer {
    class LayerStack {
    public:
        ~LayerStack();

        // Function body has to be here as it is a template function
        template <typename T, typename... Args>
        T& pushLayer(Args&&... args) {
            auto layer = std::make_unique<T>(std::forward<Args>(args)...);
            T& ref = *layer;

            layer->setLayerStack(this);
            layer->onAttach();
            this->layers.push_back(std::move(layer));

            return ref;
        }

        ILayer& pushLayer(std::unique_ptr<ILayer> layer);
        void popLayer();
        void removeLayer(ILayer& layer);
        void clear();
        void update(float dt);
        void draw();
        void onKeyPressedEvent(helios::event::KeyPressedEvent& event);
        void onKeyReleasedEvent(helios::event::KeyReleasedEvent& event);
        void onMouseButtonPressedEvent(helios::event::MouseButtonPressedEvent& event);
        void onMouseButtonReleasedEvent(helios::event::MouseButtonReleasedEvent& event);
        void onMouseMovedEvent(helios::event::MouseMovedEvent& event);
        void onMouseScrolledEvent(helios::event::MouseScrolledEvent& event);
        void onWindowClosedEvent(helios::event::WindowClosedEvent& event);
        void onWindowResizedEvent(helios::event::WindowResizedEvent& event);

    private:
        template <typename TEvent, typename TCallback>
        void dispatchEvent(TEvent& event, TCallback callback) {
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

                (layer->*callback)(event);

                if (event.handled) return;
            }
        }

        std::vector<std::unique_ptr<ILayer>> layers;
    };
}
