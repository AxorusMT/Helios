#include "Native/BallPhysicsSystem.h"

#include <Helios/ECS/Components/Components.h>
#include <Helios/Scripting/ScriptEngine.h>

#include <raylib.h>

#include <cmath>
#include <vector>

namespace {
    using helios::ecs::components::AccelerationComponent;
    using helios::ecs::components::PositionComponent;
    using helios::ecs::components::RadiusComponent;
    using helios::ecs::components::VelocityComponent;

    constexpr float ball_collision_restitution = 0.95f;
    constexpr float wall_collision_restitution = 0.88f;

    struct BallBody {
        PositionComponent* position = nullptr;
        VelocityComponent* velocity = nullptr;
        RadiusComponent* radius = nullptr;
    };

    float firstNumberOr(sol::variadic_args args, float fallback) {
        auto value = args.begin();

        if (value == args.end() || !value->is<double>()) {
            return fallback;
        }

        return static_cast<float>(value->as<double>());
    }

    void integrateBodies(helios::ecs::World& world, float dt) {
        world.each<PositionComponent, VelocityComponent, AccelerationComponent>(
            [dt](PositionComponent& position, VelocityComponent& velocity, const AccelerationComponent& acceleration) {
                velocity.x += acceleration.x * dt;
                velocity.y += acceleration.y * dt;
                position.x += velocity.x * dt;
                position.y += velocity.y * dt;
            }
        );
    }

    std::vector<BallBody> collectBalls(helios::ecs::World& world) {
        std::vector<BallBody> balls;

        world.each<PositionComponent, VelocityComponent, RadiusComponent>(
            [&balls](PositionComponent& position, VelocityComponent& velocity, RadiusComponent& radius) {
                balls.push_back(BallBody{ &position, &velocity, &radius });
            }
        );

        return balls;
    }

    void resolveBallCollisions(std::vector<BallBody>& balls) {
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
    }

    void resolveWallCollisions(helios::ecs::World& world) {
        const float width = static_cast<float>(GetScreenWidth());
        const float height = static_cast<float>(GetScreenHeight());

        if (width <= 0.0f || height <= 0.0f) {
            return;
        }

        world.each<PositionComponent, VelocityComponent, RadiusComponent>(
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

    void updateBallPhysics(helios::ecs::World& world, float dt) {
        integrateBodies(world, dt);

        std::vector<BallBody> balls = collectBalls(world);
        resolveBallCollisions(balls);
        resolveWallCollisions(world);
    }
}

void game::native::registerBallPhysicsSystem(helios::scripting::ScriptEngine& scripts) {
    helios::ecs::World& world = scripts.getWorld();

    scripts.registerNativeSystem(
        "game.balls.physics",
        [&world](sol::variadic_args args) {
            updateBallPhysics(world, firstNumberOr(args, 0.0f));
        }
    );
}
