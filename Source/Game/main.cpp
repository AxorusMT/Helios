#include "Helios/Application/Application.h"
#include "Helios/Layer/TestLayerA.h"

using namespace helios::layer;
using namespace helios::application;

int main() {
    ApplicationConfig config; 
    config.width = 1280;
    config.height = 720;
    config.title = "Helios App";
    config.target_fps = 60;
    
    Application app(config, std::make_unique<TestLayerA>());
    app.run();

    return 0;
}
