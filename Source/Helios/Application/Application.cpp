#include "Helios/Application/Application.h"
#include "Helios/Application/ApplicationControl.h"
#include "Helios/Event/Events.h"
#include "Helios/Scripting/ScriptEngine.h"

#include <raylib.h>

#include <filesystem>

namespace {
    std::filesystem::path resolveScriptRoot(const std::string& script_root) {
        std::filesystem::path configured_root(script_root);

        if (configured_root.is_absolute() || std::filesystem::exists(configured_root)) {
            return configured_root;
        }

        std::filesystem::path application_root(GetApplicationDirectory());
        std::filesystem::path executable_relative_root = application_root / configured_root;

        if (std::filesystem::exists(executable_relative_root)) {
            return executable_relative_root;
        }

        return configured_root;
    }

    void dispatchWindowEvents(helios::layer::LayerStack& layer_stack) {
        if (!IsWindowResized()) {
            return;
        }

        helios::event::WindowEvent event = helios::event::WindowEvent::resized(GetScreenWidth(), GetScreenHeight());
        layer_stack.onWindowEvent(event);
    }

    void dispatchKeyboardEvents(helios::layer::LayerStack& layer_stack) {
        for (int key = KEY_NULL + 1; key <= KEY_KB_MENU; ++key) {
            if (IsKeyDown(key)) {
                helios::event::KeyEvent event(helios::event::KeyEventAction::Held, key);
                layer_stack.onKeyEvent(event);
            }

            if (IsKeyPressed(key)) {
                helios::event::KeyEvent event(helios::event::KeyEventAction::Pressed, key);
                layer_stack.onKeyEvent(event);
            }

            if (IsKeyReleased(key)) {
                helios::event::KeyEvent event(helios::event::KeyEventAction::Released, key);
                layer_stack.onKeyEvent(event);
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
                helios::event::MouseEvent event = helios::event::MouseEvent::buttonPressed(button);
                layer_stack.onMouseEvent(event);
            }

            if (IsMouseButtonReleased(button)) {
                helios::event::MouseEvent event = helios::event::MouseEvent::buttonReleased(button);
                layer_stack.onMouseEvent(event);
            }
        }
    }

    void dispatchMouseMoveEvents(helios::layer::LayerStack& layer_stack) {
        const Vector2 delta = GetMouseDelta();

        if (delta.x == 0.0f && delta.y == 0.0f) {
            return;
        }

        const Vector2 position = GetMousePosition();
        helios::event::MouseEvent event = helios::event::MouseEvent::moved(position.x, position.y, delta.x, delta.y);
        layer_stack.onMouseEvent(event);
    }

    void dispatchMouseScrollEvents(helios::layer::LayerStack& layer_stack) {
        const Vector2 wheel_move = GetMouseWheelMoveV();

        if (wheel_move.x == 0.0f && wheel_move.y == 0.0f) {
            return;
        }

        helios::event::MouseEvent event = helios::event::MouseEvent::scrolled(wheel_move.x, wheel_move.y);
        layer_stack.onMouseEvent(event);
    }

    void dispatchMouseEvents(helios::layer::LayerStack& layer_stack) {
        dispatchMouseButtonEvents(layer_stack);
        dispatchMouseMoveEvents(layer_stack);
        dispatchMouseScrollEvents(layer_stack);
    }
}

helios::application::Application::Application(
    ApplicationConfig config,
    std::unique_ptr<helios::layer::ILayer> starting_layer
) : config(std::move(config)), starting_layer(std::move(starting_layer)), layer_stack(world) {}

helios::application::Application::~Application() {
    layer_stack.clear();
    script_engine.reset();
}

bool helios::application::Application::run() {
    clearQuitRequest();

    InitWindow(config.width, config.height, config.title.c_str());
    SetTargetFPS(config.target_fps);

    if (starting_layer != nullptr) {
        layer_stack.pushLayer(std::move(starting_layer));
    }

    if (!config.startup_script.empty()) {
        script_engine = std::make_unique<helios::scripting::ScriptEngine>(world, layer_stack);

        for (const auto& registrar : config.native_system_registrars) {
            if (registrar) {
                registrar(*script_engine);
            }
        }

        if (!script_engine->loadStartupScript(resolveScriptRoot(config.script_root), config.startup_script)) {
            layer_stack.clear();
            script_engine.reset();
            CloseWindow();
            return false;
        }
    }

    while (!WindowShouldClose() && !isQuitRequested()) {
        dispatchWindowEvents(layer_stack);
        dispatchKeyboardEvents(layer_stack);
        dispatchMouseEvents(layer_stack);

        if (isQuitRequested()) {
            break;
        }

        layer_stack.update(GetFrameTime());
        BeginDrawing();

        ClearBackground(RAYWHITE);

        //DrawText("hello, world", 40, 40, 32, BLACK);
        layer_stack.draw();
        EndDrawing();
    }

    layer_stack.clear();
    script_engine.reset();
    CloseWindow();

    return true;
}
