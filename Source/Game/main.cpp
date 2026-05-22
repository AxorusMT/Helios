#include "Helios/Application/Application.h"

int main() {
    helios::ApplicationConfig config; 
    config.width = 1280;
    config.height = 720;
    config.title = "Helios App";
    config.target_fps = 60;
    
    helios::Application app(config);
    app.run();

    return 0;
}