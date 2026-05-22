#pragma once

#include "ILayer.h"

#include <Helios/Core.h>

namespace helios::layer {
    class LayerStack {
    public:
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


        void popLayer();
        void removeLayer(ILayer& layer);
        void update(float dt);
        void draw();

    private:
        std::vector<std::unique_ptr<ILayer>> layers;
    };
}
