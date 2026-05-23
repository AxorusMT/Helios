#include <Helios/Layer/TestLayerA.h>

#include <Helios/ECS/Components/Components.h>
#include <Helios/Event/Events.h>
#include <Helios/Layer/LayerStack.h>
#include <Helios/Layer/TestLayerB.h>

#include <raylib.h>

#include <cmath>
#include <format>
#include <vector>

namespace {
    using helios::ecs::components::AccelerationComponent;
    using helios::ecs::components::ColorComponent;
    using helios::ecs::components::PositionComponent;
    using helios::ecs::components::RadiusComponent;
    using helios::ecs::components::VelocityComponent;

    struct BallBody {
        PositionComponent* position;
        VelocityComponent* velocity;
        RadiusComponent* radius;
    };
}

void helios::layer::TestLayerA::onAttach() {
    std::println("Layer A Attached!");
}

void helios::layer::TestLayerA::onDetach() {
    std::println("Layer A detached");
}

void helios::layer::TestLayerA::update(float dt) {
    getWorld().each<PositionComponent, VelocityComponent, AccelerationComponent>(
        [dt](PositionComponent& position, VelocityComponent& velocity, const AccelerationComponent& acceleration) {
            velocity.x += acceleration.x * dt;
            velocity.y += acceleration.y * dt;
            position.x += velocity.x * dt;
            position.y += velocity.y * dt;
        }
    );

    const float width = static_cast<float>(GetScreenWidth());
    const float height = static_cast<float>(GetScreenHeight());
    std::vector<BallBody> balls;

    getWorld().each<PositionComponent, VelocityComponent, RadiusComponent>(
        [&balls](PositionComponent& position, VelocityComponent& velocity, RadiusComponent& radius) {
            balls.push_back(BallBody{ &position, &velocity, &radius });
        }
    );

    for (std::size_t i = 0; i < balls.size(); ++i) {
        for (std::size_t j = i + 1; j < balls.size(); ++j) {
            BallBody& a = balls[i];
            BallBody& b = balls[j];

            const float delta_x = b.position->x - a.position->x;
            const float delta_y = b.position->y - a.position->y;
            const float min_distance = a.radius->value + b.radius->value;
            const float distance_squared = delta_x * delta_x + delta_y * delta_y;

            if (distance_squared >= min_distance * min_distance) {
                continue;
            }

            float distance = std::sqrt(distance_squared);
            float normal_x = 1.0f;
            float normal_y = 0.0f;

            if (distance > 0.0001f) {
                normal_x = delta_x / distance;
                normal_y = delta_y / distance;
            } else {
                distance = min_distance;
            }

            const float overlap = min_distance - distance;
            a.position->x -= normal_x * overlap * 0.5f;
            a.position->y -= normal_y * overlap * 0.5f;
            b.position->x += normal_x * overlap * 0.5f;
            b.position->y += normal_y * overlap * 0.5f;

            const float relative_velocity_x = b.velocity->x - a.velocity->x;
            const float relative_velocity_y = b.velocity->y - a.velocity->y;
            const float velocity_along_normal = relative_velocity_x * normal_x + relative_velocity_y * normal_y;

            if (velocity_along_normal >= 0.0f) {
                continue;
            }

            const float impulse = -(1.0f + ball_collision_restitution) * velocity_along_normal * 0.5f;
            a.velocity->x -= impulse * normal_x;
            a.velocity->y -= impulse * normal_y;
            b.velocity->x += impulse * normal_x;
            b.velocity->y += impulse * normal_y;
        }
    }

    getWorld().each<PositionComponent, VelocityComponent, RadiusComponent>(
        [width, height](PositionComponent& position, VelocityComponent& velocity, const RadiusComponent& radius) {
            if (position.x - radius.value < 0.0f) {
                position.x = radius.value;
                velocity.x = std::abs(velocity.x) * wall_collision_restitution;
            }

            if (position.x + radius.value > width) {
                position.x = width - radius.value;
                velocity.x = -std::abs(velocity.x) * wall_collision_restitution;
            }

            if (position.y - radius.value < 0.0f) {
                position.y = radius.value;
                velocity.y = std::abs(velocity.y) * wall_collision_restitution;
            }

            if (position.y + radius.value > height) {
                position.y = height - radius.value;
                velocity.y = -std::abs(velocity.y) * wall_collision_restitution;
            }
        }
    );
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

    for (int y = 0; y < height; y += 64) {
        DrawLine(0, y, width, y, Fade(SKYBLUE, 0.18f));
    }

    int ball_count = 0;

    getWorld().each<PositionComponent, ColorComponent, RadiusComponent>(
        [&ball_count](const PositionComponent& position, const ColorComponent& color, const RadiusComponent& radius) {
            DrawCircleV(
                Vector2{ position.x, position.y },
                radius.value,
                Color{ color.r, color.g, color.b, color.a }
            );

            ++ball_count;
        }
    );

    const std::string ball_count_text = std::format("Balls: {}", ball_count);
    const int font_size = 26;
    const int text_width = MeasureText(ball_count_text.c_str(), font_size);

    DrawText(
        ball_count_text.c_str(),
        (width - text_width) / 2,
        24,
        font_size,
        Color{ 35, 82, 100, 255 }
    );
}

void helios::layer::TestLayerA::spawnRandomBall() {
    const float radius = static_cast<float>(GetRandomValue(10, 32));
    const float x = static_cast<float>(GetRandomValue(static_cast<int>(radius), GetScreenWidth() - static_cast<int>(radius)));
    const float y = static_cast<float>(GetRandomValue(static_cast<int>(radius), GetScreenHeight() / 2));
    const float velocity_x = static_cast<float>(GetRandomValue(-280, 280));
    const float velocity_y = static_cast<float>(GetRandomValue(-260, 40));

    helios::ecs::Entity ball = getWorld().createEntity();
    ball.addComponent<PositionComponent>(PositionComponent{ x, y });
    ball.addComponent<RadiusComponent>(RadiusComponent{ radius });
    ball.addComponent<VelocityComponent>(VelocityComponent{ velocity_x, velocity_y });
    ball.addComponent<AccelerationComponent>(AccelerationComponent{ 0.0f, gravity });
    ball.addComponent<ColorComponent>(
        ColorComponent{
            static_cast<uint8_t>(GetRandomValue(64, 255)),
            static_cast<uint8_t>(GetRandomValue(64, 255)),
            static_cast<uint8_t>(GetRandomValue(64, 255)),
            255
        }
    );
}

void helios::layer::TestLayerA::removeBall() {
    helios::ecs::Entity ball_to_remove;

    getWorld().each<PositionComponent, ColorComponent, RadiusComponent>(
        [&ball_to_remove](helios::ecs::Entity entity, PositionComponent&, ColorComponent&, RadiusComponent&) {
            if (!ball_to_remove.isValid()) {
                ball_to_remove = entity;
            }
        }
    );

    if (ball_to_remove.isValid()) {
        ball_to_remove.destroy();
    }
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
        case KEY_UP:
            spawnRandomBall();
            event.handled = true;
            break;
        case KEY_DOWN:
            removeBall();
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
