#include "Helios/Application/Application.h"
#include "Helios/Layer/TestLayerA.h"
#include "Helios/Layer/TestLayerB.h"
#include <raylib.h>

bool helios::Application::run() {
    InitWindow(config.width, config.height, config.title.c_str());
    SetTargetFPS(config.target_fps);

    layer_stack.pushLayer<helios::layer::TestLayerB>();

    while (!WindowShouldClose()) {
        layer_stack.update(GetFrameTime());
        BeginDrawing();

        ClearBackground(RAYWHITE);

        //DrawText("hello, world", 40, 40, 32, BLACK);
        layer_stack.draw();
        EndDrawing();
    }

    CloseWindow();

    return true;
}