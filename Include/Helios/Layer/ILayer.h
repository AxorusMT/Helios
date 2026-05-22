#pragma once

#include <Helios/Core.h>

namespace helios::layer {
    class ILayer {
    public:
        virtual ~ILayer() = default;

        // These funcs are not pure virtual because not all layers override all funcs
        virtual void onAttach() {}       
        virtual void onDetach() {}
        virtual void update(float dt) {}
        virtual void draw() {} 

    };
}   