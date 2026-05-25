#include "Native/BallPhysicsSystem.h"

#include <Helios/ECS/Components/Components.h>
#include <Helios/ECS/Entity.h>
#include <Helios/Scripting/ScriptEngine.h>

#include <raylib.h>
#include <rlgl.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <memory>
#include <vector>

namespace {
    using helios::ecs::components::AccelerationComponent;
    using helios::ecs::components::ColorComponent;
    using helios::ecs::components::PositionComponent;
    using helios::ecs::components::RadiusComponent;
    using helios::ecs::components::VelocityComponent;

    constexpr float ball_collision_restitution = 0.95f;
    constexpr float wall_collision_restitution = 0.88f;
    constexpr float gravity = 330.0f;
    constexpr float collision_cell_size = 32.0f;
    constexpr float velocity_epsilon = 0.0001f;
    constexpr int ball_texture_size = 64;
    constexpr std::size_t high_count_collision_threshold = 12000u;
    constexpr std::size_t high_count_collision_pair_budget = 120000u;
    constexpr std::size_t high_count_cell_pair_budget = 96u;
    constexpr std::size_t exact_cell_pair_limit = 256u;
    constexpr std::size_t outline_render_limit = 5000u;
    constexpr float paddle_height = 24.0f;
    constexpr float paddle_bottom_margin = 54.0f;
    constexpr float pulse_radius = 185.0f;
    constexpr float pulse_strength = 620.0f;
    constexpr float pulse_cooldown_duration = 1.05f;
    constexpr int starting_ball_count = 7;

    struct BallBody {
        helios::ecs::Entity entity;
        PositionComponent* position = nullptr;
        VelocityComponent* velocity = nullptr;
        RadiusComponent* radius = nullptr;
        ColorComponent* color = nullptr;
        float inverse_mass = 1.0f;
        int min_cell_x = 0;
        int min_cell_y = 0;
        int max_cell_x = 0;
        int max_cell_y = 0;
    };

    struct CollisionGridCell {
        int x = 0;
        int y = 0;
        std::vector<std::size_t> body_indices;
    };

    struct CollisionGrid {
        int min_cell_x = 0;
        int min_cell_y = 0;
        int width = 0;
        int height = 0;
        std::vector<CollisionGridCell> cells;
    };

    struct BallSystemState {
        std::vector<BallBody> balls;
        CollisionGrid collision_grid;
        std::vector<helios::ecs::Entity> pending_destroy;
        bool cache_valid = false;
        uint32_t collision_phase = 0;
        bool initialized = false;
        bool paused = false;
        bool game_over = false;
        bool pulse_queued = false;
        int score = 0;
        int lives = 5;
        int wave = 1;
        int combo = 0;
        int best_combo = 0;
        int caught = 0;
        int missed = 0;
        float elapsed = 0.0f;
        float spawn_timer = 0.0f;
        float paddle_x = 640.0f;
        float pulse_cooldown = 0.0f;
        float pulse_flash = 0.0f;
    };

    void resetGame(helios::ecs::World& world, BallSystemState& state);
    void flushPendingDestroy(BallSystemState& state);
    float paddleWidth(const BallSystemState& state);
    float paddleY();
    void updatePlayer(BallSystemState& state, float dt);
    void spawnGameWave(helios::ecs::World& world, BallSystemState& state, int count);
    void updateSpawner(helios::ecs::World& world, BallSystemState& state, float dt);

    float firstNumberOr(sol::variadic_args args, float fallback) {
        auto value = args.begin();

        if (value == args.end() || !value->is<double>()) {
            return fallback;
        }

        return static_cast<float>(value->as<double>());
    }

    bool firstBoolOr(sol::variadic_args args, bool fallback) {
        auto value = args.begin();

        if (value == args.end() || !value->is<bool>()) {
            return fallback;
        }

        return value->as<bool>();
    }

    int firstIntOr(sol::variadic_args args, int fallback) {
        auto value = args.begin();

        if (value == args.end()) {
            return fallback;
        }

        if (value->is<int>()) {
            return value->as<int>();
        }

        if (value->is<double>()) {
            return static_cast<int>(value->as<double>());
        }

        return fallback;
    }

    float inverseMassFromRadius(const RadiusComponent& radius) {
        const float clamped_radius = radius.value > 0.0001f ? radius.value : 0.0001f;
        const float mass = clamped_radius * clamped_radius;
        return 1.0f / mass;
    }

    int cellForCoordinate(float coordinate) {
        return static_cast<int>(std::floor(coordinate / collision_cell_size));
    }

    void assignCollisionCells(BallBody& body) {
        const float radius = std::max(body.radius->value, 0.0f);
        body.min_cell_x = cellForCoordinate(body.position->x - radius);
        body.min_cell_y = cellForCoordinate(body.position->y - radius);
        body.max_cell_x = cellForCoordinate(body.position->x + radius);
        body.max_cell_y = cellForCoordinate(body.position->y + radius);
    }

    void integrateAndCollectBalls(helios::ecs::World& world, std::vector<BallBody>& balls, float dt) {
        balls.clear();
        world.each<PositionComponent, VelocityComponent, RadiusComponent>(
            [&balls, dt](
                helios::ecs::Entity entity,
                PositionComponent& position,
                VelocityComponent& velocity,
                RadiusComponent& radius
            ) {
                const AccelerationComponent* acceleration = entity.tryGetComponent<AccelerationComponent>();

                if (acceleration != nullptr) {
                    velocity.x += acceleration->x * dt;
                    velocity.y += acceleration->y * dt;
                    position.x += velocity.x * dt;
                    position.y += velocity.y * dt;
                }

                BallBody& body = balls.emplace_back();
                body.entity = entity;
                body.position = &position;
                body.velocity = &velocity;
                body.radius = &radius;
                body.color = entity.tryGetComponent<ColorComponent>();
                body.inverse_mass = inverseMassFromRadius(radius);
                assignCollisionCells(body);
            }
        );
    }

    void buildCollisionGrid(
        const std::vector<BallBody>& balls,
        CollisionGrid& grid
    ) {
        if (balls.empty()) {
            grid.width = 0;
            grid.height = 0;
            grid.cells.clear();
            return;
        }

        int min_cell_x = balls.front().min_cell_x;
        int min_cell_y = balls.front().min_cell_y;
        int max_cell_x = balls.front().max_cell_x;
        int max_cell_y = balls.front().max_cell_y;

        for (const BallBody& body : balls) {
            min_cell_x = std::min(min_cell_x, body.min_cell_x);
            min_cell_y = std::min(min_cell_y, body.min_cell_y);
            max_cell_x = std::max(max_cell_x, body.max_cell_x);
            max_cell_y = std::max(max_cell_y, body.max_cell_y);
        }

        grid.min_cell_x = min_cell_x;
        grid.min_cell_y = min_cell_y;
        grid.width = max_cell_x - min_cell_x + 1;
        grid.height = max_cell_y - min_cell_y + 1;

        const std::size_t cell_count = static_cast<std::size_t>(grid.width) * static_cast<std::size_t>(grid.height);
        grid.cells.resize(cell_count);

        for (int y = 0; y < grid.height; ++y) {
            for (int x = 0; x < grid.width; ++x) {
                CollisionGridCell& cell = grid.cells[static_cast<std::size_t>(y) * grid.width + x];
                cell.x = grid.min_cell_x + x;
                cell.y = grid.min_cell_y + y;
                cell.body_indices.clear();
            }
        }

        for (std::size_t index = 0; index < balls.size(); ++index) {
            const BallBody& body = balls[index];

            for (int cell_y = body.min_cell_y; cell_y <= body.max_cell_y; ++cell_y) {
                for (int cell_x = body.min_cell_x; cell_x <= body.max_cell_x; ++cell_x) {
                    const int local_x = cell_x - grid.min_cell_x;
                    const int local_y = cell_y - grid.min_cell_y;
                    CollisionGridCell& cell = grid.cells[static_cast<std::size_t>(local_y) * grid.width + local_x];
                    cell.body_indices.push_back(index);
                }
            }
        }
    }

    bool ownsBodyPair(const CollisionGridCell& cell, const BallBody& a, const BallBody& b) {
        return cell.x == std::max(a.min_cell_x, b.min_cell_x)
            && cell.y == std::max(a.min_cell_y, b.min_cell_y);
    }

    void resolveBallCollision(BallBody& a, BallBody& b) {
        const float delta_x = b.position->x - a.position->x;
        const float delta_y = b.position->y - a.position->y;
        const float min_distance = a.radius->value + b.radius->value;
        const float distance_squared = delta_x * delta_x + delta_y * delta_y;

        if (distance_squared >= min_distance * min_distance) {
            return;
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
        const float inverse_mass_sum = a.inverse_mass + b.inverse_mass;

        if (inverse_mass_sum <= 0.0f) {
            return;
        }

        const float a_correction_weight = a.inverse_mass / inverse_mass_sum;
        const float b_correction_weight = b.inverse_mass / inverse_mass_sum;

        a.position->x -= normal_x * overlap * a_correction_weight;
        a.position->y -= normal_y * overlap * a_correction_weight;
        b.position->x += normal_x * overlap * b_correction_weight;
        b.position->y += normal_y * overlap * b_correction_weight;

        const float relative_velocity_x = b.velocity->x - a.velocity->x;
        const float relative_velocity_y = b.velocity->y - a.velocity->y;
        const float velocity_along_normal = relative_velocity_x * normal_x + relative_velocity_y * normal_y;

        if (velocity_along_normal >= 0.0f) {
            return;
        }

        const float impulse = -(1.0f + ball_collision_restitution) * velocity_along_normal / inverse_mass_sum;
        a.velocity->x -= impulse * a.inverse_mass * normal_x;
        a.velocity->y -= impulse * a.inverse_mass * normal_y;
        b.velocity->x += impulse * b.inverse_mass * normal_x;
        b.velocity->y += impulse * b.inverse_mass * normal_y;
    }

    void resolveCellCollisionsExact(
        std::vector<BallBody>& balls,
        const CollisionGridCell& cell,
        std::size_t& remaining_pair_budget
    ) {
        const std::vector<std::size_t>& body_indices = cell.body_indices;

        for (std::size_t i = 0; i < body_indices.size(); ++i) {
            for (std::size_t j = i + 1; j < body_indices.size(); ++j) {
                BallBody& a = balls[body_indices[i]];
                BallBody& b = balls[body_indices[j]];

                if (!ownsBodyPair(cell, a, b)) {
                    continue;
                }

                resolveBallCollision(a, b);

                if (remaining_pair_budget != std::numeric_limits<std::size_t>::max()) {
                    --remaining_pair_budget;

                    if (remaining_pair_budget == 0u) {
                        return;
                    }
                }
            }
        }
    }

    void resolveCellCollisionsSampled(
        std::vector<BallBody>& balls,
        const CollisionGridCell& cell,
        uint32_t phase,
        std::size_t& remaining_pair_budget
    ) {
        const std::vector<std::size_t>& body_indices = cell.body_indices;
        const std::size_t body_count = body_indices.size();

        if (body_count < 2u || remaining_pair_budget == 0u) {
            return;
        }

        const std::size_t sample_count = std::min({ high_count_cell_pair_budget, remaining_pair_budget, body_count * 4u });
        const std::size_t phase_offset = static_cast<std::size_t>(phase) % body_count;
        const std::size_t stride = ((static_cast<std::size_t>(phase) * 17u) + 5u) % (body_count - 1u) + 1u;

        for (std::size_t sample = 0; sample < sample_count; ++sample) {
            const std::size_t i = (phase_offset + sample) % body_count;
            std::size_t j = (i + 1u + ((sample * stride) % (body_count - 1u))) % body_count;

            if (j == i) {
                j = (j + 1u) % body_count;
            }

            BallBody& a = balls[body_indices[std::min(i, j)]];
            BallBody& b = balls[body_indices[std::max(i, j)]];

            if (ownsBodyPair(cell, a, b)) {
                resolveBallCollision(a, b);
            }

            --remaining_pair_budget;

            if (remaining_pair_budget == 0u) {
                return;
            }
        }
    }

    void resolveBallCollisions(
        std::vector<BallBody>& balls,
        CollisionGrid& grid,
        uint32_t phase
    ) {
        buildCollisionGrid(balls, grid);

        const bool budgeted = balls.size() > high_count_collision_threshold;
        std::size_t remaining_pair_budget = budgeted
            ? high_count_collision_pair_budget
            : std::numeric_limits<std::size_t>::max();

        for (const CollisionGridCell& cell : grid.cells) {
            const std::size_t body_count = cell.body_indices.size();

            if (body_count < 2u) {
                continue;
            }

            const std::size_t cell_pair_count = body_count * (body_count - 1u) / 2u;

            if (!budgeted || cell_pair_count <= exact_cell_pair_limit) {
                resolveCellCollisionsExact(balls, cell, remaining_pair_budget);
            } else {
                resolveCellCollisionsSampled(balls, cell, phase, remaining_pair_budget);
            }

            if (remaining_pair_budget == 0u) {
                return;
            }
        }
    }

    void resolveWallCollisions(std::vector<BallBody>& balls) {
        const float width = static_cast<float>(GetScreenWidth());

        if (width <= 0.0f) {
            return;
        }

        for (BallBody& body : balls) {
            PositionComponent& position = *body.position;
            VelocityComponent& velocity = *body.velocity;
            const RadiusComponent& radius = *body.radius;

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
        }
    }

    void applyPulse(BallSystemState& state) {
        if (!state.pulse_queued || state.pulse_cooldown > 0.0f || state.game_over || state.paused) {
            state.pulse_queued = false;
            return;
        }

        const Vector2 origin = GetMousePosition();
        const float radius_squared = pulse_radius * pulse_radius;

        for (BallBody& body : state.balls) {
            const float delta_x = body.position->x - origin.x;
            const float delta_y = body.position->y - origin.y;
            const float distance_squared = delta_x * delta_x + delta_y * delta_y;

            if (distance_squared > radius_squared) {
                continue;
            }

            const float distance = std::sqrt(std::max(distance_squared, 1.0f));
            const float falloff = 1.0f - std::min(distance / pulse_radius, 1.0f);
            const float impulse = pulse_strength * (0.35f + falloff * 0.65f);

            body.velocity->x += (delta_x / distance) * impulse;
            body.velocity->y += (delta_y / distance) * impulse - impulse * 0.45f;
        }

        state.pulse_queued = false;
        state.pulse_cooldown = pulse_cooldown_duration;
        state.pulse_flash = 0.18f;
    }

    void handleCatchesAndMisses(BallSystemState& state) {
        const float height = static_cast<float>(GetScreenHeight());

        if (height <= 0.0f) {
            return;
        }

        const float catcher_y = paddleY();
        const float half_width = paddleWidth(state) * 0.5f;
        const float catcher_left = state.paddle_x - half_width;
        const float catcher_right = state.paddle_x + half_width;

        for (BallBody& body : state.balls) {
            PositionComponent& position = *body.position;
            VelocityComponent& velocity = *body.velocity;
            const RadiusComponent& radius = *body.radius;

            if (velocity.y >= 0.0f
                && position.y + radius.value >= catcher_y - paddle_height * 0.5f
                && position.y - radius.value <= catcher_y + paddle_height * 0.75f
                && position.x + radius.value >= catcher_left
                && position.x - radius.value <= catcher_right
            ) {
                state.pending_destroy.push_back(body.entity);
                ++state.caught;
                ++state.combo;
                state.best_combo = std::max(state.best_combo, state.combo);
                state.score += 10 * state.wave + state.combo;

                if (state.caught % 35 == 0) {
                    ++state.wave;
                }

                continue;
            }

            if (position.y - radius.value > height + 36.0f) {
                state.pending_destroy.push_back(body.entity);
                ++state.missed;
                state.combo = 0;

                if (state.lives > 0) {
                    --state.lives;
                }

                if (state.lives <= 0) {
                    state.game_over = true;
                }
            }
        }
    }

    void updateBallPhysics(helios::ecs::World& world, BallSystemState& state, float dt) {
        if (!state.initialized && IsWindowReady()) {
            resetGame(world, state);
            spawnGameWave(world, state, starting_ball_count);
        }

        if (IsKeyPressed(KEY_R)) {
            resetGame(world, state);
            spawnGameWave(world, state, starting_ball_count);
        }

        if (IsKeyPressed(KEY_P)) {
            state.paused = !state.paused;
        }

        if (IsKeyPressed(KEY_X)) {
            state.pulse_queued = true;
        }

        flushPendingDestroy(state);
        updatePlayer(state, dt);

        state.pulse_cooldown = std::max(0.0f, state.pulse_cooldown - dt);
        state.pulse_flash = std::max(0.0f, state.pulse_flash - dt);

        if (state.paused || state.game_over) {
            integrateAndCollectBalls(world, state.balls, 0.0f);
            state.cache_valid = true;
            return;
        }

        state.elapsed += dt;
        updateSpawner(world, state, dt);
        integrateAndCollectBalls(world, state.balls, dt);
        applyPulse(state);
        resolveBallCollisions(state.balls, state.collision_grid, state.collision_phase++);
        resolveWallCollisions(state.balls);
        handleCatchesAndMisses(state);
        state.cache_valid = true;
    }

    Color randomBallColor() {
        return Color{
            static_cast<unsigned char>(GetRandomValue(64, 255)),
            static_cast<unsigned char>(GetRandomValue(64, 255)),
            static_cast<unsigned char>(GetRandomValue(64, 255)),
            255
        };
    }

    helios::ecs::Entity spawnBall(
        helios::ecs::World& world,
        float x,
        float y,
        float radius,
        float velocity_x,
        float velocity_y,
        Color color
    ) {
        helios::ecs::Entity ball = world.createEntity();
        ball.addComponent<PositionComponent>(PositionComponent{ x, y });
        ball.addComponent<RadiusComponent>(RadiusComponent{ radius });
        ball.addComponent<VelocityComponent>(VelocityComponent{ velocity_x, velocity_y });
        ball.addComponent<AccelerationComponent>(AccelerationComponent{ 0.0f, gravity });
        ball.addComponent<ColorComponent>(ColorComponent{ color.r, color.g, color.b, color.a });
        return ball;
    }

    void clearBalls(helios::ecs::World& world) {
        std::vector<helios::ecs::Entity> balls_to_remove;

        world.each<PositionComponent, ColorComponent, RadiusComponent>(
            [&balls_to_remove](helios::ecs::Entity entity, PositionComponent&, ColorComponent&, RadiusComponent&) {
                balls_to_remove.push_back(entity);
            }
        );

        for (helios::ecs::Entity& ball : balls_to_remove) {
            if (ball.isValid()) {
                ball.destroy();
            }
        }
    }

    void resetGame(helios::ecs::World& world, BallSystemState& state) {
        clearBalls(world);
        state.balls.clear();
        state.collision_grid.cells.clear();
        state.pending_destroy.clear();
        state.cache_valid = false;
        state.initialized = true;
        state.paused = false;
        state.game_over = false;
        state.pulse_queued = false;
        state.score = 0;
        state.lives = 5;
        state.wave = 1;
        state.combo = 0;
        state.best_combo = 0;
        state.caught = 0;
        state.missed = 0;
        state.elapsed = 0.0f;
        state.spawn_timer = 0.0f;
        state.paddle_x = static_cast<float>(GetScreenWidth()) * 0.5f;
        state.pulse_cooldown = 0.0f;
        state.pulse_flash = 0.0f;
    }

    void flushPendingDestroy(BallSystemState& state) {
        for (helios::ecs::Entity& entity : state.pending_destroy) {
            if (entity.isValid()) {
                entity.destroy();
            }
        }

        state.pending_destroy.clear();
    }

    float paddleWidth(const BallSystemState& state) {
        return std::max(160.0f, 286.0f - static_cast<float>(state.wave - 1) * 4.0f);
    }

    float paddleY() {
        return static_cast<float>(GetScreenHeight()) - paddle_bottom_margin;
    }

    void updatePlayer(BallSystemState& state, float dt) {
        const float width = static_cast<float>(GetScreenWidth());

        if (width <= 0.0f) {
            return;
        }

        const float keyboard_speed = 720.0f;
        float target_x = static_cast<float>(GetMouseX());

        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
            target_x = state.paddle_x - keyboard_speed * dt;
        }

        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
            target_x = state.paddle_x + keyboard_speed * dt;
        }

        const float half_width = paddleWidth(state) * 0.5f;
        state.paddle_x = std::clamp(target_x, half_width, width - half_width);
    }

    void spawnGameWave(helios::ecs::World& world, BallSystemState& state, int count) {
        const int width = GetScreenWidth();

        if (width <= 0 || count <= 0) {
            return;
        }

        for (int index = 0; index < count; ++index) {
            const int radius = GetRandomValue(6, 12);
            const int x = GetRandomValue(radius, std::max(radius, width - radius));
            const int y = GetRandomValue(-180, -radius);
            const int velocity_x = GetRandomValue(-110 - state.wave * 5, 110 + state.wave * 5);
            const int velocity_y = GetRandomValue(0, 72 + state.wave * 6);

            spawnBall(
                world,
                static_cast<float>(x),
                static_cast<float>(y),
                static_cast<float>(radius),
                static_cast<float>(velocity_x),
                static_cast<float>(velocity_y),
                randomBallColor()
            );
        }
    }

    void updateSpawner(helios::ecs::World& world, BallSystemState& state, float dt) {
        state.spawn_timer += dt;

        const std::size_t active_ball_limit = static_cast<std::size_t>(std::min(52, 18 + state.wave * 3));
        const float interval = std::max(0.32f, 1.12f - static_cast<float>(state.wave - 1) * 0.035f);

        while (state.spawn_timer >= interval) {
            state.spawn_timer -= interval;

            if (state.balls.size() >= active_ball_limit) {
                continue;
            }

            const int spawn_count = 1 + std::min(2, state.wave / 6);
            spawnGameWave(world, state, spawn_count);
        }
    }

    void spawnRandomBalls(helios::ecs::World& world, int count) {
        const int width = GetScreenWidth();
        const int height = GetScreenHeight();

        if (width <= 0 || height <= 0 || count <= 0) {
            return;
        }

        for (int index = 0; index < count; ++index) {
            const int radius = GetRandomValue(5, 10);
            const int x = GetRandomValue(radius, std::max(radius, width - radius));
            const int y = GetRandomValue(radius, std::max(radius, height / 2));
            const int velocity_x = GetRandomValue(-280, 280);
            const int velocity_y = GetRandomValue(-260, 40);

            spawnBall(
                world,
                static_cast<float>(x),
                static_cast<float>(y),
                static_cast<float>(radius),
                static_cast<float>(velocity_x),
                static_cast<float>(velocity_y),
                randomBallColor()
            );
        }
    }

    void spawnCursorBall(helios::ecs::World& world, float radius) {
        const Vector2 position = GetMousePosition();
        spawnBall(world, position.x, position.y, radius, 0.0f, 0.0f, randomBallColor());
    }

    void removeBalls(helios::ecs::World& world, int count) {
        if (count <= 0) {
            return;
        }

        std::vector<helios::ecs::Entity> balls_to_remove;
        balls_to_remove.reserve(static_cast<std::size_t>(count));

        world.each<PositionComponent, ColorComponent, RadiusComponent>(
            [&balls_to_remove, count](helios::ecs::Entity entity, PositionComponent&, ColorComponent&, RadiusComponent&) {
                if (static_cast<int>(balls_to_remove.size()) < count) {
                    balls_to_remove.push_back(entity);
                }
            }
        );

        for (helios::ecs::Entity& ball : balls_to_remove) {
            if (ball.isValid()) {
                ball.destroy();
            }
        }
    }

    Color componentColor(const ColorComponent& color) {
        return Color{ color.r, color.g, color.b, color.a };
    }

    struct BallTexture {
        Texture2D texture{};
        bool loaded = false;
    };

    BallTexture& getBallTexture() {
        static BallTexture ball_texture;
        return ball_texture;
    }

    void ensureBallTextureLoaded() {
        BallTexture& ball_texture = getBallTexture();

        if (ball_texture.loaded || !IsWindowReady()) {
            return;
        }

        Image image = GenImageColor(ball_texture_size, ball_texture_size, BLANK);
        ImageDrawCircle(
            &image,
            ball_texture_size / 2,
            ball_texture_size / 2,
            (ball_texture_size / 2) - 1,
            WHITE
        );

        ball_texture.texture = LoadTextureFromImage(image);
        SetTextureFilter(ball_texture.texture, TEXTURE_FILTER_BILINEAR);
        UnloadImage(image);
        ball_texture.loaded = true;
    }

    void drawBallFill(const PositionComponent& position, const RadiusComponent& radius, Color color) {
        ensureBallTextureLoaded();

        const BallTexture& ball_texture = getBallTexture();

        if (!ball_texture.loaded) {
            DrawCircleV(Vector2{ position.x, position.y }, radius.value, color);
            return;
        }

        const float diameter = radius.value * 2.0f;
        DrawTexturePro(
            ball_texture.texture,
            Rectangle{ 0.0f, 0.0f, static_cast<float>(ball_texture_size), static_cast<float>(ball_texture_size) },
            Rectangle{ position.x - radius.value, position.y - radius.value, diameter, diameter },
            Vector2{ 0.0f, 0.0f },
            0.0f,
            color
        );
    }

    int drawBallFillBatch(const std::vector<BallBody>& balls) {
        ensureBallTextureLoaded();

        const BallTexture& ball_texture = getBallTexture();

        if (!ball_texture.loaded) {
            return -1;
        }

        int ball_count = 0;

        rlSetTexture(ball_texture.texture.id);
        rlBegin(RL_QUADS);

        for (const BallBody& ball : balls) {
            if (ball.color == nullptr) {
                continue;
            }

            const float left = ball.position->x - ball.radius->value;
            const float top = ball.position->y - ball.radius->value;
            const float right = ball.position->x + ball.radius->value;
            const float bottom = ball.position->y + ball.radius->value;
            const Color color = componentColor(*ball.color);

            rlColor4ub(color.r, color.g, color.b, color.a);
            rlNormal3f(0.0f, 0.0f, 1.0f);

            rlTexCoord2f(0.0f, 0.0f);
            rlVertex2f(left, top);

            rlTexCoord2f(0.0f, 1.0f);
            rlVertex2f(left, bottom);

            rlTexCoord2f(1.0f, 1.0f);
            rlVertex2f(right, bottom);

            rlTexCoord2f(1.0f, 0.0f);
            rlVertex2f(right, top);

            ++ball_count;
        }

        rlEnd();
        rlSetTexture(0);

        return ball_count;
    }

    void drawLine(float x1, float y1, float x2, float y2, Color color) {
        DrawLine(
            static_cast<int>(std::round(x1)),
            static_cast<int>(std::round(y1)),
            static_cast<int>(std::round(x2)),
            static_cast<int>(std::round(y2)),
            color
        );
    }

    void drawVelocityLine(
        const PositionComponent& position,
        const VelocityComponent& velocity,
        const RadiusComponent& radius
    ) {
        const float velocity_length = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);

        if (velocity_length <= velocity_epsilon) {
            return;
        }

        const float direction_x = velocity.x / velocity_length;
        const float direction_y = velocity.y / velocity_length;
        const float end_x = position.x + direction_x * radius.value;
        const float end_y = position.y + direction_y * radius.value;
        const float perpendicular_x = -direction_y;
        const float perpendicular_y = direction_x;
        const float arrow_head_length = std::min(radius.value * 0.45f, 12.0f);
        const float arrow_head_width = arrow_head_length * 0.55f;
        constexpr Color velocity_color{ 0, 80, 255, 255 };

        drawLine(position.x, position.y, end_x, end_y, velocity_color);
        drawLine(
            end_x,
            end_y,
            end_x - direction_x * arrow_head_length + perpendicular_x * arrow_head_width,
            end_y - direction_y * arrow_head_length + perpendicular_y * arrow_head_width,
            velocity_color
        );
        drawLine(
            end_x,
            end_y,
            end_x - direction_x * arrow_head_length - perpendicular_x * arrow_head_width,
            end_y - direction_y * arrow_head_length - perpendicular_y * arrow_head_width,
            velocity_color
        );
    }

    void drawBallOutline(
        const PositionComponent& position,
        const RadiusComponent& radius,
        const VelocityComponent* velocity
    ) {
        constexpr Color outline_color{ 0, 0, 0, 255 };

        DrawCircleLines(
            static_cast<int>(std::round(position.x)),
            static_cast<int>(std::round(position.y)),
            radius.value,
            outline_color
        );

        if (velocity != nullptr) {
            drawVelocityLine(position, *velocity, radius);
        }
    }

    void drawBallOutline(helios::ecs::Entity entity) {
        if (!entity.isValid()) {
            return;
        }

        const PositionComponent* position = entity.tryGetComponent<PositionComponent>();
        const RadiusComponent* radius = entity.tryGetComponent<RadiusComponent>();

        if (position == nullptr || radius == nullptr) {
            return;
        }

        drawBallOutline(*position, *radius, entity.tryGetComponent<VelocityComponent>());
    }

    void drawBallOutlines(helios::ecs::World& world) {
        world.each<PositionComponent, RadiusComponent>(
            [](helios::ecs::Entity entity, PositionComponent& position, RadiusComponent& radius) {
                drawBallOutline(position, radius, entity.tryGetComponent<VelocityComponent>());
            }
        );
    }

    int drawCachedBalls(const std::vector<BallBody>& balls, bool draw_outline) {
        if (!draw_outline) {
            const int batched_ball_count = drawBallFillBatch(balls);

            if (batched_ball_count >= 0) {
                return batched_ball_count;
            }
        }

        int ball_count = 0;

        for (const BallBody& ball : balls) {
            if (ball.color == nullptr) {
                continue;
            }

            drawBallFill(*ball.position, *ball.radius, componentColor(*ball.color));

            if (draw_outline) {
                drawBallOutline(*ball.position, *ball.radius, ball.velocity);
            }

            ++ball_count;
        }

        return ball_count;
    }

    int drawBallsFromWorld(helios::ecs::World& world, bool draw_outline) {
        int ball_count = 0;

        world.each<PositionComponent, ColorComponent, RadiusComponent>(
            [&ball_count, draw_outline](
                helios::ecs::Entity entity,
                PositionComponent& position,
                ColorComponent& color,
                RadiusComponent& radius
            ) {
                drawBallFill(position, radius, componentColor(color));

                if (draw_outline) {
                    drawBallOutline(position, radius, entity.tryGetComponent<VelocityComponent>());
                }

                ++ball_count;
            }
        );

        return ball_count;
    }

    int drawBalls(helios::ecs::World& world, BallSystemState& state, bool draw_outline) {
        if (state.cache_valid) {
            const bool effective_draw_outline = draw_outline && state.balls.size() <= outline_render_limit;
            return drawCachedBalls(state.balls, effective_draw_outline);
        }

        return drawBallsFromWorld(world, draw_outline);
    }

    void drawPaddle(const BallSystemState& state) {
        const float width = paddleWidth(state);
        const float y = paddleY();
        const Rectangle catcher{
            state.paddle_x - width * 0.5f,
            y - paddle_height * 0.5f,
            width,
            paddle_height
        };

        DrawRectangleRounded(catcher, 0.55f, 14, Color{ 26, 42, 54, 255 });
        DrawRectangleRoundedLinesEx(catcher, 0.55f, 14, 3.0f, Color{ 99, 219, 255, 255 });

        const float cooldown_fill = state.pulse_cooldown <= 0.0f
            ? 1.0f
            : 1.0f - std::clamp(state.pulse_cooldown / pulse_cooldown_duration, 0.0f, 1.0f);
        const Rectangle meter_bg{ catcher.x, catcher.y + catcher.height + 8.0f, catcher.width, 5.0f };
        const Rectangle meter_fg{ meter_bg.x, meter_bg.y, meter_bg.width * cooldown_fill, meter_bg.height };

        DrawRectangleRec(meter_bg, Color{ 33, 63, 76, 95 });
        DrawRectangleRec(meter_fg, state.pulse_cooldown <= 0.0f ? Color{ 255, 222, 89, 255 } : Color{ 99, 219, 255, 210 });

        if (state.pulse_flash > 0.0f) {
            const Vector2 mouse = GetMousePosition();
            const float alpha = std::clamp(state.pulse_flash / 0.18f, 0.0f, 1.0f);
            DrawCircleLines(
                static_cast<int>(std::round(mouse.x)),
                static_cast<int>(std::round(mouse.y)),
                pulse_radius * (1.0f - alpha * 0.35f),
                Fade(Color{ 255, 222, 89, 255 }, alpha)
            );
        }
    }

    void drawHud(const BallSystemState& state, int ball_count) {
        char left_text[160]{};
        std::snprintf(
            left_text,
            sizeof(left_text),
            "Score %d   Wave %d   Combo %d",
            state.score,
            state.wave,
            state.combo
        );

        char right_text[128]{};
        std::snprintf(
            right_text,
            sizeof(right_text),
            "Lives %d   Balls %d",
            state.lives,
            ball_count
        );

        DrawText(left_text, 24, 22, 24, Color{ 35, 82, 100, 255 });

        const int right_width = MeasureText(right_text, 24);
        DrawText(right_text, GetScreenWidth() - right_width - 24, 22, 24, Color{ 35, 82, 100, 255 });

        if (state.paused || state.game_over) {
            const char* title = state.game_over ? "GAME OVER" : "PAUSED";
            char subtitle[128]{};
            std::snprintf(
                subtitle,
                sizeof(subtitle),
                state.game_over ? "R to restart   Best combo %d" : "P to resume   R to restart",
                state.best_combo
            );

            constexpr int title_size = 52;
            constexpr int subtitle_size = 24;
            const int title_width = MeasureText(title, title_size);
            const int subtitle_width = MeasureText(subtitle, subtitle_size);
            const int center_x = GetScreenWidth() / 2;
            const int center_y = GetScreenHeight() / 2;

            DrawRectangle(0, center_y - 95, GetScreenWidth(), 164, Fade(Color{ 8, 18, 30, 255 }, 0.62f));
            DrawText(title, center_x - title_width / 2, center_y - 72, title_size, Color{ 245, 252, 255, 255 });
            DrawText(subtitle, center_x - subtitle_width / 2, center_y - 4, subtitle_size, Color{ 191, 231, 241, 255 });
        }
    }

    void drawBallsSystem(helios::ecs::World& world, BallSystemState& state, sol::variadic_args args) {
        const int ball_count = drawBalls(world, state, firstBoolOr(args, false));
        drawPaddle(state);
        drawHud(state, ball_count);
    }

    void drawBallOutlineSystem(helios::ecs::World& world, sol::variadic_args args) {
        auto value = args.begin();

        if (value != args.end() && value->is<helios::ecs::Entity>()) {
            drawBallOutline(value->as<helios::ecs::Entity>());
            return;
        }

        drawBallOutlines(world);
    }

    void queuePulse(BallSystemState& state) {
        state.pulse_queued = true;
    }
}

void game::native::registerBallPhysicsSystem(helios::scripting::ScriptEngine& scripts) {
    helios::ecs::World& world = scripts.getWorld();
    auto state = std::make_shared<BallSystemState>();

    scripts.registerNativeSystem(
        "game.balls.physics",
        [&world, state](sol::variadic_args args) {
            updateBallPhysics(world, *state, firstNumberOr(args, 0.0f));
        }
    );
    scripts.registerNativeSystem(
        "game.balls.draw_outline",
        [&world](sol::variadic_args args) {
            drawBallOutlineSystem(world, args);
        }
    );
    scripts.registerNativeSystem(
        "game.balls.draw",
        [&world, state](sol::variadic_args args) {
            drawBallsSystem(world, *state, args);
        }
    );
    scripts.registerNativeSystem(
        "game.balls.spawn_random",
        [&world, state](sol::variadic_args args) {
            spawnRandomBalls(world, firstIntOr(args, 1));
            state->cache_valid = false;
        }
    );
    scripts.registerNativeSystem(
        "game.balls.spawn_cursor",
        [&world, state](sol::variadic_args args) {
            spawnCursorBall(world, firstNumberOr(args, 100.0f));
            state->cache_valid = false;
        }
    );
    scripts.registerNativeSystem(
        "game.balls.remove",
        [&world, state](sol::variadic_args args) {
            removeBalls(world, firstIntOr(args, 1));
            state->cache_valid = false;
        }
    );
    scripts.registerNativeSystem(
        "game.balls.pulse",
        [state](sol::variadic_args) {
            queuePulse(*state);
        }
    );
}
