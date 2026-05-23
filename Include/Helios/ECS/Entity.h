#pragma once

#include <Helios/Core.h>

#include <entt/entt.hpp>

namespace helios::ecs {
    class World;

    class Entity {
    public:
        Entity() = default;

        bool isValid() const;
        void destroy();

        template <typename TComponent, typename... Args>
        TComponent& addComponent(Args&&... args);

        template <typename TComponent, typename... Args>
        TComponent& addOrReplaceComponent(Args&&... args);

        template <typename TComponent>
        TComponent& getComponent();

        template <typename TComponent>
        TComponent* tryGetComponent();

        template <typename TComponent>
        bool hasComponent() const;

        template <typename TComponent>
        void removeComponent();

    private:
        friend class World;

        Entity(World& world, entt::entity handle) : world(&world), handle(handle) {}

        World* world = nullptr;
        entt::entity handle = entt::null;
    };
}
