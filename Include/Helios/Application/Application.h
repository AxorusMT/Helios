#pragma once

#include "Helios/Core.h"
#include "Helios/Application/ApplicationConfig.h"
#include "Helios/ECS/ECS.h"
#include "Helios/Layer/LayerStack.h"
#include "Helios/Layer/ILayer.h"

namespace helios::application {
    class Application {
    public:
        Application(ApplicationConfig config, std::unique_ptr<helios::layer::ILayer> starting_layer)
            : config(std::move(config)), starting_layer(std::move(starting_layer)), layer_stack(world) {}

        // todo: error codes
        bool run();

    private:
        ApplicationConfig config;
        std::unique_ptr<helios::layer::ILayer> starting_layer;
        helios::ecs::World world;
        helios::layer::LayerStack layer_stack;
    };
}
