#pragma once

#include "Helios/Core.h"
#include "Helios/Application/ApplicationConfig.h"
#include "Helios/ECS/ECS.h"
#include "Helios/Layer/LayerStack.h"
#include "Helios/Layer/ILayer.h"

namespace helios::scripting {
    class ScriptEngine;
}

namespace helios::application {
    class Application {
    public:
        explicit Application(ApplicationConfig config, std::unique_ptr<helios::layer::ILayer> starting_layer = nullptr);
        ~Application();

        // todo: error codes
        bool run();

    private:
        ApplicationConfig config;
        std::unique_ptr<helios::layer::ILayer> starting_layer;
        helios::ecs::World world;
        helios::layer::LayerStack layer_stack;
        std::unique_ptr<helios::scripting::ScriptEngine> script_engine;
    };
}
