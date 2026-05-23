#include <Helios/Layer/TestLayerA.h>

#include <Helios/Event/Key/KeyPressedEvent.h>
#include <Helios/Event/Key/KeyReleasedEvent.h>
#include <Helios/Layer/LayerStack.h>
#include <Helios/Layer/TestLayerB.h>

#include <raylib.h>

void helios::layer::TestLayerA::onAttach() {
    std::println("Layer A Attached!");
}

void helios::layer::TestLayerA::onDetach() {
    std::println("Layer A detached");
}

void helios::layer::TestLayerA::draw() {
    const int width = GetScreenWidth();
    const int height = GetScreenHeight();

    DrawRectangleGradientV(
        0,
        0,
        width,
        height,
        Color{ 220, 242, 255, 255 },
        Color{ 218, 248, 224, 255 }
    );

    for (int x = 0; x < width; x += 64) {
        DrawLine(x, 0, x, height, Fade(SKYBLUE, 0.25f));
    }

    DrawCircle(width - 96, 88, 46, Color{ 255, 203, 72, 255 });
    DrawCircleLines(width - 96, 88, 56, Color{ 236, 164, 45, 255 });

    DrawRectangle(48, 64, 380, 118, Fade(WHITE, 0.78f));
    DrawRectangleLinesEx(Rectangle{ 48, 64, 380, 118 }, 3.0f, Color{ 57, 120, 89, 255 });
    DrawText(this->name.c_str(), 68, 82, 34, Color{ 35, 82, 62, 255 });
    DrawText("Base layer: WASD logs input", 70, 126, 20, Color{ 54, 93, 74, 255 });
    DrawText("Hold SPACE to show Layer B", 70, 152, 20, Color{ 54, 93, 74, 255 });

    DrawRectangle(90, height - 138, 190, 78, Color{ 89, 176, 118, 255 });
    DrawRectangleLinesEx(Rectangle{ 90, static_cast<float>(height - 138), 190, 78 }, 4.0f, Color{ 38, 111, 69, 255 });
    DrawCircle(380, height - 96, 52, Color{ 80, 156, 216, 255 });
    DrawTriangle(
        Vector2{ static_cast<float>(width - 210), static_cast<float>(height - 64) },
        Vector2{ static_cast<float>(width - 132), static_cast<float>(height - 170) },
        Vector2{ static_cast<float>(width - 54), static_cast<float>(height - 64) },
        Color{ 236, 117, 86, 255 }
    );
}

void helios::layer::TestLayerA::onKeyPressedEvent(helios::event::KeyPressedEvent& event) {
    switch (event.getKeyCode()) {
        case KEY_W:
            std::println("w");
            event.handled = true;
            break;
        case KEY_A:
            std::println("a");
            event.handled = true;
            break;
        case KEY_S:
            std::println("s");
            event.handled = true;
            break;
        case KEY_D:
            std::println("d");
            event.handled = true;
            break;
        case KEY_SPACE:
            if (test_layer_b == nullptr) {
                test_layer_b = &getLayerStack().pushLayer<TestLayerB>();
            }

            event.handled = true;
            break;
    }
}

void helios::layer::TestLayerA::onKeyReleasedEvent(helios::event::KeyReleasedEvent& event) {
    if (event.getKeyCode() != KEY_SPACE || test_layer_b == nullptr) return;

    getLayerStack().removeLayer(*test_layer_b);
    test_layer_b = nullptr;
    event.handled = true;
}
