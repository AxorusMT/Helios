#pragma once

#include <Helios/Core.h>

#include "Helios/Layer/ILayer.h"

namespace game::layer {
    class TestLayerB : public helios::layer::ILayer {
    public:
        void onAttach() override;
        void onDetach() override;
        void draw() override;
        void onKeyEvent(helios::event::KeyEvent& event) override;
    private:
        std::string name = "TestLayerB";
    };
}
