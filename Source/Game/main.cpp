#include "Helios/Application/Application.h"
#include "Native/BallPhysicsSystem.h"

using namespace helios::application;

int main() {
    ApplicationConfig config; 
    config.width = 1280;
    config.height = 720;
    config.title = "Helios App";
    config.target_fps = 60;
    config.script_root = "Scripts";
    config.startup_script = "main.lua";
    config.native_system_registrars.push_back(game::native::registerBallPhysicsSystem);
    
    Application app(config);
    app.run();

    return 0;
}
