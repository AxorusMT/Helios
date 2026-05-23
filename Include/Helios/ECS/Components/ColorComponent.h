#pragma once

#include <Helios/Core.h>

namespace helios::ecs::components {
    struct ColorComponent {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
}
