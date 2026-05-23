#pragma once

#include <Helios/Core.h>

#include "Helios/Layer/ILayer.h"

namespace game::layer {
    class TestLayerB;

    class TestLayerA : public helios::layer::ILayer {
    public:
        void onAttach() override;
        void onDetach() override;
        void update(float dt) override;
        void draw() override;
        void onKeyHeldEvent(helios::event::KeyHeldEvent& event) override;
        void onKeyPressedEvent(helios::event::KeyPressedEvent& event) override;
        void onKeyReleasedEvent(helios::event::KeyReleasedEvent& event) override;
        void onMouseButtonPressedEvent(helios::event::MouseButtonPressedEvent& event) override;
        void onMouseButtonReleasedEvent(helios::event::MouseButtonReleasedEvent& event) override;
        void onMouseMovedEvent(helios::event::MouseMovedEvent& event) override;

    private:
        void spawnRandomBall();
        void removeBall();
        void startDraggingBall(float mouse_x, float mouse_y);
        void dragBall(float mouse_x, float mouse_y, float delta_x, float delta_y);
        void stopDraggingBall();

        static constexpr float gravity = 420.0f;
        static constexpr float ball_collision_restitution = 0.95f;
        static constexpr float ball_action_interval = 0.035f;
        static constexpr float drag_velocity_scale = 18.0f;
        static constexpr float wall_collision_restitution = 0.88f;

        std::string name = "TestLayerA";
        TestLayerB* test_layer_b = nullptr;
        helios::ecs::Entity dragged_ball;
        float drag_offset_x = 0.0f;
        float drag_offset_y = 0.0f;
        float ball_action_timer = 0.0f;
    };
}
