#pragma once

#include <Helios/Core.h>

#include "Helios/Layer/ILayer.h"

namespace game::layer {
    class TestLayerB : public helios::layer::ILayer {
    public:
        void onAttach() override;
        void onDetach() override;
        void draw() override;
        void onKeyHeldEvent(helios::event::KeyHeldEvent& event) override;
        void onKeyPressedEvent(helios::event::KeyPressedEvent& event) override;
    private:
        std::string name = "TestLayerB";
    };
}
