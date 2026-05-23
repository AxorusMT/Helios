#pragma once

#include <Helios/Core.h>

#include "Helios/Layer/ILayer.h"

namespace helios::layer {
    class TestLayerB : public ILayer {
    public:
        void onAttach() override;
        void onDetach() override;
        void draw() override;
        void onKeyPressedEvent(helios::event::KeyPressedEvent& event) override;
    private:
        std::string name = "TestLayerB";
    };
}
