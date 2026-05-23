#pragma once

#include <Helios/Core.h>
#include <Helios/Event/Key/KeyPressedEvent.h>
#include <Helios/Event/Key/KeyReleasedEvent.h>
#include <Helios/Event/Mouse/MouseButtonPressedEvent.h>
#include <Helios/Event/Mouse/MouseButtonReleasedEvent.h>
#include <Helios/Event/Mouse/MouseMovedEvent.h>
#include <Helios/Event/Mouse/MouseScrolledEvent.h>
#include <Helios/Event/Window/WindowClosedEvent.h>
#include <Helios/Event/Window/WindowResizedEvent.h>

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
        virtual void onKeyPressedEvent(helios::event::KeyPressedEvent& event) {}
        virtual void onKeyReleasedEvent(helios::event::KeyReleasedEvent& event) {}
        virtual void onMouseButtonPressedEvent(helios::event::MouseButtonPressedEvent& event) {}
        virtual void onMouseButtonReleasedEvent(helios::event::MouseButtonReleasedEvent& event) {}
        virtual void onMouseMovedEvent(helios::event::MouseMovedEvent& event) {}
        virtual void onMouseScrolledEvent(helios::event::MouseScrolledEvent& event) {}
        virtual void onWindowClosedEvent(helios::event::WindowClosedEvent& event) {}
        virtual void onWindowResizedEvent(helios::event::WindowResizedEvent& event) {}

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
