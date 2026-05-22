#pragma once

#include <Helios/Core.h>

#include "Helios/Layer/ILayer.h"

namespace helios::layer {
    class TestLayerB : public ILayer {
    public:
        void onAttach() override;
        void onDetach() override;
        void update(float dt) override;
        void draw() override;
    private:
        std::string name = "TestLayerB";
    };
}