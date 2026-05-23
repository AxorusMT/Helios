#include "Helios/Application/Application.h"
#include "Helios/Event/KeyEvent.h"
#include "Helios/Layer/TestLayerA.h"
#include <raylib.h>

namespace {
    void dispatchKeyboardEvents(helios::layer::LayerStack& layer_stack) {
        for (int key = KEY_NULL + 1; key <= KEY_KB_MENU; ++key) {
            if (IsKeyPressed(key)) {
                helios::event::KeyPressed event(key);
                layer_stack.onEvent(event);
            }

            if (IsKeyReleased(key)) {
                helios::event::KeyReleased event(key);
                layer_stack.onEvent(event);
            }
        }
    }
}

bool helios::Application::run() {
    InitWindow(config.width, config.height, config.title.c_str());
    SetTargetFPS(config.target_fps);

    layer_stack.pushLayer<helios::layer::TestLayerA>();

    while (!WindowShouldClose()) {
        dispatchKeyboardEvents(layer_stack);
        layer_stack.update(GetFrameTime());
        BeginDrawing();

        ClearBackground(RAYWHITE);

        //DrawText("hello, world", 40, 40, 32, BLACK);
        layer_stack.draw();
        EndDrawing();
    }

    layer_stack.clear();
    CloseWindow();

    return true;
}
