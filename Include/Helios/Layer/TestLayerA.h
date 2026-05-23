#pragma once

#include <Helios/Core.h>

#include "Helios/Layer/ILayer.h"

namespace helios::layer {
    class TestLayerB;

    class TestLayerA : public ILayer {
    public:
        void onAttach() override;
        void onDetach() override;
        void draw() override;
        void onKeyPressedEvent(helios::event::KeyPressedEvent& event) override;
        void onKeyReleasedEvent(helios::event::KeyReleasedEvent& event) override;
    private:
        std::string name = "TestLayerA";
        TestLayerB* test_layer_b = nullptr;
    };
}
