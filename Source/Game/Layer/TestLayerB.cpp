#include "Layer/TestLayerB.h"

#include <Helios/Event/Events.h>

#include <raylib.h>

void game::layer::TestLayerB::onAttach() {
    std::println("Layer B Attached!");
}

void game::layer::TestLayerB::onDetach() {
    std::println("Layer B detached");
}

void game::layer::TestLayerB::draw() {
    const int width = GetScreenWidth();
    const int height = GetScreenHeight();
    const float center_x = static_cast<float>(width) * 0.5f;
    const float center_y = static_cast<float>(height) * 0.5f;

    DrawRectangle(0, 0, width, height, Fade(Color{ 18, 24, 48, 255 }, 0.72f));

    DrawCircleV(Vector2{ center_x - 190.0f, center_y - 98.0f }, 70.0f, Fade(Color{ 255, 80, 140, 255 }, 0.86f));
    DrawTriangle(
        Vector2{ center_x + 168.0f, center_y - 168.0f },
        Vector2{ center_x + 256.0f, center_y - 28.0f },
        Vector2{ center_x + 80.0f, center_y - 28.0f },
        Fade(Color{ 115, 232, 255, 255 }, 0.9f)
    );

    const Rectangle panel = Rectangle{ center_x - 260.0f, center_y - 92.0f, 520.0f, 184.0f };
    DrawRectangleRec(panel, Color{ 246, 248, 255, 242 });
    DrawRectangleLinesEx(panel, 5.0f, Color{ 78, 93, 199, 255 });

    DrawText(this->name.c_str(), static_cast<int>(panel.x + 34.0f), static_cast<int>(panel.y + 28.0f), 46, Color{ 45, 54, 134, 255 });
    DrawText("Overlay layer: arrow keys log input", static_cast<int>(panel.x + 36.0f), static_cast<int>(panel.y + 88.0f), 22, Color{ 68, 75, 118, 255 });
    DrawText("Release SPACE to remove this overlay", static_cast<int>(panel.x + 36.0f), static_cast<int>(panel.y + 120.0f), 22, Color{ 68, 75, 118, 255 });

    DrawCircleLines(static_cast<int>(center_x - 120.0f), static_cast<int>(center_y + 144.0f), 48.0f, Color{ 255, 222, 89, 255 });
    DrawRectangle(static_cast<int>(center_x + 86.0f), static_cast<int>(center_y + 116.0f), 112, 58, Color{ 255, 222, 89, 255 });
}

void game::layer::TestLayerB::onKeyHeldEvent(helios::event::KeyHeldEvent& event) {
    switch (event.getKeyCode()) {
        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
            event.handled = true;
            break;
    }
}

void game::layer::TestLayerB::onKeyPressedEvent(helios::event::KeyPressedEvent& event) {
    switch (event.getKeyCode()) {
        case KEY_UP:
            std::println("^");
            event.handled = true;
            break;
        case KEY_DOWN:
            std::println("v");
            event.handled = true;
            break;
        case KEY_LEFT:
            std::println("<");
            event.handled = true;
            break;
        case KEY_RIGHT:
            std::println(">");
            event.handled = true;
            break;
    }
}
