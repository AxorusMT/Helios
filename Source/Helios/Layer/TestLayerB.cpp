#include <Helios/Layer/TestLayerB.h>

#include <raylib.h>

void helios::layer::TestLayerB::onAttach() {
    std::println("Layer B Attached!");
}

void helios::layer::TestLayerB::onDetach() {
    std::println("Layer B detached");
}

void helios::layer::TestLayerB::update(float dt) {
    if (IsKeyPressed(KEY_UP)) std::println("^");
    if (IsKeyPressed(KEY_DOWN)) std::println("v");
    if (IsKeyPressed(KEY_LEFT)) std::println("<");
    if (IsKeyPressed(KEY_RIGHT)) std::println(">");
}

void helios::layer::TestLayerB::draw() {
    DrawText(this->name.c_str(), 80, 200, 48, BLUE);
}