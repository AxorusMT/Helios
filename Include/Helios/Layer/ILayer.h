#pragma once

#include <Helios/Core.h>

#include <cassert>

namespace helios::layer {
    class LayerStack;

    class ILayer {
    public:
        virtual ~ILayer() = default;

        // These funcs are not pure virtual because not all layers override all funcs
        virtual void onAttach() {}       
        virtual void onDetach() {}
        virtual void update(float dt) {}
        virtual void draw() {} 

    protected:
        LayerStack& getLayerStack() {
            assert(layer_stack != nullptr);
            return *layer_stack;
        }

    private:
        friend class LayerStack;

        void setLayerStack(LayerStack* stack) {
            layer_stack = stack;
        }

        LayerStack* layer_stack = nullptr;
    };
}   
