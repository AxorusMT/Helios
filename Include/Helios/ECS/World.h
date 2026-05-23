#pragma once

#include <Helios/Core.h>
#include <Helios/ECS/Entity.h>

#include <cassert>
#include <functional>
#include <type_traits>

namespace helios::ecs {
    namespace detail {
        template <typename...>
        inline constexpr bool always_false_v = false;
    }

    class World {
    public:
        Entity createEntity() {
            return Entity(*this, registry.create());
        }

        void destroyEntity(Entity entity) {
            assert(entity.world == this);
            assert(isValid(entity));

            registry.destroy(entity.handle);
        }

        bool isValid(Entity entity) const {
            return entity.world == this
                && entity.handle != entt::null
                && registry.valid(entity.handle);
        }

        template <typename... TComponents, typename TCallback>
        void each(TCallback&& callback) {
            static_assert(sizeof...(TComponents) > 0, "World::each requires at least one component type");

            auto view = registry.view<TComponents...>();

            for (entt::entity handle : view) {
                Entity entity(*this, handle);

                if constexpr (std::is_invocable_v<TCallback&, Entity, TComponents&...>) {
                    std::invoke(callback, entity, view.get<TComponents>(handle)...);
                } else if constexpr (std::is_invocable_v<TCallback&, TComponents&...>) {
                    std::invoke(callback, view.get<TComponents>(handle)...);
                } else {
                    static_assert(
                        detail::always_false_v<TCallback, TComponents...>,
                        "World::each callback must accept either (Entity, Components&...) or (Components&...)"
                    );
                }
            }
        }

    private:
        friend class Entity;

        entt::registry registry;
    };

    inline bool Entity::isValid() const {
        return world != nullptr && world->isValid(*this);
    }

    inline void Entity::destroy() {
        assert(isValid());

        world->destroyEntity(*this);
        world = nullptr;
        handle = entt::null;
    }

    template <typename TComponent, typename... Args>
    TComponent& Entity::addComponent(Args&&... args) {
        assert(isValid());
        assert(!hasComponent<TComponent>());

        return world->registry.emplace<TComponent>(handle, std::forward<Args>(args)...);
    }

    template <typename TComponent, typename... Args>
    TComponent& Entity::addOrReplaceComponent(Args&&... args) {
        assert(isValid());

        return world->registry.emplace_or_replace<TComponent>(handle, std::forward<Args>(args)...);
    }

    template <typename TComponent>
    TComponent& Entity::getComponent() {
        assert(isValid());
        assert(hasComponent<TComponent>());

        return world->registry.get<TComponent>(handle);
    }

    template <typename TComponent>
    TComponent* Entity::tryGetComponent() {
        if (!isValid()) {
            return nullptr;
        }

        return world->registry.try_get<TComponent>(handle);
    }

    template <typename TComponent>
    bool Entity::hasComponent() const {
        return isValid() && world->registry.all_of<TComponent>(handle);
    }

    template <typename TComponent>
    void Entity::removeComponent() {
        assert(isValid());
        assert(hasComponent<TComponent>());

        world->registry.remove<TComponent>(handle);
    }
}
