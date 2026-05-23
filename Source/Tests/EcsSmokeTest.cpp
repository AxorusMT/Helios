#include <Helios/ECS/ECS.h>

#include <cassert>

namespace {
    struct Position {
        float x;
        float y;
    };

    struct Velocity {
        float x;
        float y;
    };

    struct Health {
        int value;
    };
}

int main() {
    helios::ecs::World world;

    helios::ecs::Entity entity = world.createEntity();
    assert(entity.isValid());
    assert(world.isValid(entity));
    assert(!entity.hasComponent<Position>());
    assert(entity.tryGetComponent<Position>() == nullptr);

    Position& position = entity.addComponent<Position>(1.0f, 2.0f);
    assert(entity.hasComponent<Position>());
    assert(entity.tryGetComponent<Position>() == &position);
    assert(entity.getComponent<Position>().x == 1.0f);
    assert(entity.getComponent<Position>().y == 2.0f);

    entity.addOrReplaceComponent<Position>(3.0f, 4.0f);
    assert(entity.getComponent<Position>().x == 3.0f);
    assert(entity.getComponent<Position>().y == 4.0f);

    entity.removeComponent<Position>();
    assert(!entity.hasComponent<Position>());
    assert(entity.tryGetComponent<Position>() == nullptr);

    helios::ecs::Entity moving_entity = world.createEntity();
    moving_entity.addComponent<Position>(10.0f, 20.0f);
    moving_entity.addComponent<Velocity>(1.0f, 2.0f);

    helios::ecs::Entity static_entity = world.createEntity();
    static_entity.addComponent<Position>(30.0f, 40.0f);
    static_entity.addComponent<Health>(100);

    int moving_count = 0;
    world.each<Position, Velocity>(
        [&moving_count](helios::ecs::Entity iterated_entity, Position& iterated_position, Velocity& iterated_velocity) {
            assert(iterated_entity.isValid());
            iterated_position.x += iterated_velocity.x;
            iterated_position.y += iterated_velocity.y;
            ++moving_count;
        }
    );

    assert(moving_count == 1);
    assert(moving_entity.getComponent<Position>().x == 11.0f);
    assert(moving_entity.getComponent<Position>().y == 22.0f);
    assert(static_entity.getComponent<Position>().x == 30.0f);
    assert(static_entity.getComponent<Position>().y == 40.0f);

    int position_count = 0;
    world.each<Position>(
        [&position_count](Position&) {
            ++position_count;
        }
    );

    assert(position_count == 2);

    world.destroyEntity(static_entity);
    assert(!world.isValid(static_entity));
    assert(!static_entity.isValid());

    moving_entity.destroy();
    assert(!moving_entity.isValid());

    return 0;
}
