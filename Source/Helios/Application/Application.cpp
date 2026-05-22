#include "Helios/Application/Application.h"
#include <raylib.h>

bool helios::Application::run() {
    InitWindow(config.width, config.height, config.title.c_str());
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText("hello, world", 40, 40, 32, BLACK);

        EndDrawing();
    }

    CloseWindow();

    return true;
}