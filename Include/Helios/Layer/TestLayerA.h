#pragma once

#include <Helios/Core.h>

#include "Helios/Layer/ILayer.h"

namespace helios::layer {
    class TestLayerB;

    class TestLayerA : public ILayer {
    public:
        void onAttach() override;
        void onDetach() override;
        void update(float dt) override;
        void draw() override;
        void onKeyPressedEvent(helios::event::KeyPressedEvent& event) override;
        void onKeyReleasedEvent(helios::event::KeyReleasedEvent& event) override;

    private:
        void spawnRandomBall();
        void removeBall();

        static constexpr float gravity = 420.0f;
        static constexpr float ball_collision_restitution = 0.95f;
        static constexpr float wall_collision_restitution = 0.88f;

        std::string name = "TestLayerA";
        TestLayerB* test_layer_b = nullptr;
    };
}
