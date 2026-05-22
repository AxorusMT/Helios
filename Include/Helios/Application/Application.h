#pragma once

#include "Helios/Core.h"
#include "Helios/Application/ApplicationConfig.h"
#include "Helios/Layer/LayerStack.h"

namespace helios {
    class Application {
    public:
        Application(ApplicationConfig config) : config(config) {};
        // todo: error codes
        bool run();
    private:
        ApplicationConfig config;
        helios::layer::LayerStack layer_stack;
    };
}