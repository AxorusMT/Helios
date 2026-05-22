#include <Helios/Layer/TestLayerA.h>

#include <raylib.h>

void helios::layer::TestLayerA::onAttach() {
    std::println("Layer A Attached!");
}

void helios::layer::TestLayerA::onDetach() {
    std::println("Layer A detached");
}

void helios::layer::TestLayerA::update(float dt) {
    if (IsKeyPressed(KEY_W)) std::println("w");
    if (IsKeyPressed(KEY_A)) std::println("a");
    if (IsKeyPressed(KEY_S)) std::println("s");
    if (IsKeyPressed(KEY_D)) std::println("d");
}

void helios::layer::TestLayerA::draw() {
    DrawText(this->name.c_str(), 40, 80, 24, DARKGRAY);
}