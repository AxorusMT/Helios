#include "Helios/Helios.h"

#include <raylib.h>
#include <cstdint>

void helios::Application::run() {
    constexpr uint32_t width = 1280;
    constexpr uint32_t height = 720;
    
    InitWindow(width, height, "Helios Application");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText("hello, world", 40, 40, 32, BLACK);

        EndDrawing();
    }

    CloseWindow();

}