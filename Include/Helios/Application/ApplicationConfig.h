#pragma once

#include "Helios/Core.h"

namespace helios::application {
    struct ApplicationConfig {
        std::string title;
        uint32_t width;
        uint32_t height;
        uint16_t target_fps;
    };
}