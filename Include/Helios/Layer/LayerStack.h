#pragma once

#include "ILayer.h"

#include <Helios/Core.h>
#include <Helios/ECS/ECS.h>
#include <Helios/Layer/LayerHandle.h>

namespace helios::layer {
    class LayerStack {
    public:
        explicit LayerStack(helios::ecs::World& world) : world(world) {}
        ~LayerStack();

        // Function body has to be here as it is a template function
        template <typename T, typename... Args>
        T& pushLayer(Args&&... args) {
            auto layer = std::make_unique<T>(std::forward<Args>(args)...);
            T& ref = *layer;

            layer->setLayerContext(this, &world);
            layer->onAttach();
            this->layers.push_back(std::move(layer));

            return ref;
        }

        ILayer& pushLayer(std::unique_ptr<ILayer> layer);
        void popLayer();
        void removeLayer(ILayer& layer);
        bool removeLayer(LayerHandle handle);
        [[nodiscard]] bool containsLayer(LayerHandle handle) const;
        void clear();
        void update(float dt);
        void draw();
        void onKeyHeldEvent(helios::event::KeyHeldEvent& event);
        void onKeyPressedEvent(helios::event::KeyPressedEvent& event);
        void onKeyReleasedEvent(helios::event::KeyReleasedEvent& event);
        void onMouseButtonPressedEvent(helios::event::MouseButtonPressedEvent& event);
        void onMouseButtonReleasedEvent(helios::event::MouseButtonReleasedEvent& event);
        void onMouseMovedEvent(helios::event::MouseMovedEvent& event);
        void onMouseScrolledEvent(helios::event::MouseScrolledEvent& event);
        void onWindowClosedEvent(helios::event::WindowClosedEvent& event);
        void onWindowResizedEvent(helios::event::WindowResizedEvent& event);

    private:
        [[nodiscard]] ILayer* findLayer(LayerHandle handle) const;

        template <typename TEvent, typename TCallback>
        void dispatchEvent(TEvent& event, TCallback callback) {
            std::vector<LayerHandle> dispatch_layers;
            dispatch_layers.reserve(layers.size());

            for (auto it = layers.rbegin(); it != layers.rend(); ++it) {
                dispatch_layers.push_back((*it)->getLayerHandle());
            }

            for (LayerHandle handle : dispatch_layers) {
                ILayer* layer = findLayer(handle);
                if (layer == nullptr) continue;

                (layer->*callback)(event);

                if (event.handled) return;
            }
        }

        helios::ecs::World& world;
        std::vector<std::unique_ptr<ILayer>> layers;
        uint64_t next_layer_id = 1;
    };
}
