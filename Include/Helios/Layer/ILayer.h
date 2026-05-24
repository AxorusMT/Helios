#pragma once

#include <Helios/Core.h>
#include <Helios/ECS/ECS.h>
#include <Helios/Event/Events.h>
#include <Helios/Layer/LayerHandle.h>

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
        virtual void onKeyEvent(helios::event::KeyEvent& event) {}
        virtual void onMouseEvent(helios::event::MouseEvent& event) {}
        virtual void onWindowEvent(helios::event::WindowEvent& event) {}

        [[nodiscard]] LayerHandle getLayerHandle() const {
            return layer_handle;
        }

    protected:
        LayerStack& getLayerStack() {
            assert(layer_stack != nullptr);
            return *layer_stack;
        }

        helios::ecs::World& getWorld() {
            assert(world != nullptr);
            return *world;
        }

    private:
        friend class LayerStack;

        void setLayerContext(LayerStack* stack, helios::ecs::World* shared_world) {
            layer_stack = stack;
            world = shared_world;
        }

        void setLayerHandle(LayerHandle handle) {
            layer_handle = handle;
        }

        LayerStack* layer_stack = nullptr;
        helios::ecs::World* world = nullptr;
        LayerHandle layer_handle;
    };
}   
