#pragma once

#include "Helios/Core.h"

#include <functional>

namespace helios::scripting {
    class ScriptEngine;
}

namespace helios::application {
    using NativeSystemRegistrar = std::function<void(helios::scripting::ScriptEngine&)>;

    struct ApplicationConfig {
        std::string title = "Helios App";
        uint32_t width = 1280;
        uint32_t height = 720;
        uint16_t target_fps = 60;
        std::string script_root = "Scripts";
        std::string startup_script = "main.lua";
        std::vector<NativeSystemRegistrar> native_system_registrars;
    };
}
