#include "Helios/Application/Application.h"
#include "Helios/Event/Events.h"
#include "Helios/Layer/TestLayerA.h"
#include <raylib.h>

namespace {
    void dispatchWindowEvents(helios::layer::LayerStack& layer_stack) {
        if (!IsWindowResized()) {
            return;
        }

        helios::event::WindowResizedEvent event(GetScreenWidth(), GetScreenHeight());
        layer_stack.onWindowResizedEvent(event);
    }

    void dispatchKeyboardEvents(helios::layer::LayerStack& layer_stack) {
        for (int key = KEY_NULL + 1; key <= KEY_KB_MENU; ++key) {
            if (IsKeyPressed(key)) {
                helios::event::KeyPressedEvent event(key);
                layer_stack.onKeyPressedEvent(event);
            }

            if (IsKeyReleased(key)) {
                helios::event::KeyReleasedEvent event(key);
                layer_stack.onKeyReleasedEvent(event);
            }
        }
    }

    void dispatchMouseButtonEvents(helios::layer::LayerStack& layer_stack) {
        constexpr int mouse_buttons[] = {
            MOUSE_BUTTON_LEFT,
            MOUSE_BUTTON_RIGHT,
            MOUSE_BUTTON_MIDDLE,
            MOUSE_BUTTON_SIDE,
            MOUSE_BUTTON_EXTRA,
            MOUSE_BUTTON_FORWARD,
            MOUSE_BUTTON_BACK
        };

        for (const int button : mouse_buttons) {
            if (IsMouseButtonPressed(button)) {
                helios::event::MouseButtonPressedEvent event(button);
                layer_stack.onMouseButtonPressedEvent(event);
            }

            if (IsMouseButtonReleased(button)) {
                helios::event::MouseButtonReleasedEvent event(button);
                layer_stack.onMouseButtonReleasedEvent(event);
            }
        }
    }

    void dispatchMouseMoveEvents(helios::layer::LayerStack& layer_stack) {
        const Vector2 delta = GetMouseDelta();

        if (delta.x == 0.0f && delta.y == 0.0f) {
            return;
        }

        const Vector2 position = GetMousePosition();
        helios::event::MouseMovedEvent event(position.x, position.y, delta.x, delta.y);
        layer_stack.onMouseMovedEvent(event);
    }

    void dispatchMouseScrollEvents(helios::layer::LayerStack& layer_stack) {
        const Vector2 wheel_move = GetMouseWheelMoveV();

        if (wheel_move.x == 0.0f && wheel_move.y == 0.0f) {
            return;
        }

        helios::event::MouseScrolledEvent event(wheel_move.x, wheel_move.y);
        layer_stack.onMouseScrolledEvent(event);
    }

    void dispatchMouseEvents(helios::layer::LayerStack& layer_stack) {
        dispatchMouseButtonEvents(layer_stack);
        dispatchMouseMoveEvents(layer_stack);
        dispatchMouseScrollEvents(layer_stack);
    }
}

bool helios::Application::run() {
    InitWindow(config.width, config.height, config.title.c_str());
    SetTargetFPS(config.target_fps);

    layer_stack.pushLayer<helios::layer::TestLayerA>();

    while (!WindowShouldClose()) {
        dispatchWindowEvents(layer_stack);
        dispatchKeyboardEvents(layer_stack);
        dispatchMouseEvents(layer_stack);
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
