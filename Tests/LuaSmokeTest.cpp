#include <Helios/ECS/ECS.h>
#include <Helios/Event/KeyEvent.h>
#include <Helios/Event/MouseEvent.h>
#include <Helios/Layer/LayerStack.h>
#include <Helios/Scripting/ScriptEngine.h>

#include "Native/BallPhysicsSystem.h"

#include <cassert>
#include <cmath>
#include <filesystem>

namespace {
    double tableNumber(sol::table table, const char* key) {
        return table.get_or(key, 0.0);
    }

    bool tableBool(sol::table table, const char* key) {
        return table.get_or(key, false);
    }
}

int main() {
    helios::ecs::World world;
    helios::layer::LayerStack layer_stack(world);
    helios::scripting::ScriptEngine scripts(world, layer_stack);

    assert(
        scripts.runString(
            R"(
                key_constants_check = {
                    o = type(helios.key.o) == "number"
                }
            )",
            "key_constants_smoke"
        )
    );
    assert(tableBool(scripts.lua()["key_constants_check"], "o"));

    assert(
        scripts.runString(
            R"(
                events = {}

                handle = helios.layers.push({
                    name = "SmokeLayer",
                    on_attach = function(self)
                        table.insert(events, "attach")
                    end,
                    update = function(self, dt)
                        self.dt = dt
                        table.insert(events, "update")
                    end,
                    draw = function(self)
                        table.insert(events, "draw")
                    end,
                    on_detach = function(self)
                        table.insert(events, "detach")
                    end
                })

                empty_handle = helios.layers.push({ name = "EmptyLayer" })
                removed_empty = helios.layers.remove(empty_handle)
            )",
            "layer_lifecycle_smoke"
        )
    );

    assert(scripts.isLayerValid(scripts.lua()["handle"].get<helios::layer::LayerHandle>()));
    assert(scripts.lua()["removed_empty"].get<bool>());

    layer_stack.update(0.25f);
    layer_stack.draw();
    layer_stack.clear();

    sol::table events = scripts.lua()["events"];
    assert(events[1].get<std::string>() == "attach");
    assert(events[2].get<std::string>() == "update");
    assert(events[3].get<std::string>() == "draw");
    assert(events[4].get<std::string>() == "detach");

    assert(
        scripts.runString(
            R"(
                lower_called = false
                upper_called = false

                helios.layers.push({
                    on_key_event = function(self, event)
                        lower_called = true
                    end
                })

                helios.layers.push({
                    on_key_event = function(self, event)
                        if event.action == "pressed" then
                            upper_called = true
                            event.handled = true
                        end
                    end
                })
            )",
            "event_handled_smoke"
        )
    );

    helios::event::KeyEvent key_event(helios::event::KeyEventAction::Pressed, 123);
    layer_stack.onKeyEvent(key_event);
    assert(key_event.handled);
    assert(scripts.lua()["upper_called"].get<bool>());
    assert(!scripts.lua()["lower_called"].get<bool>());
    layer_stack.clear();

    assert(
        scripts.runString(
            R"(
                local entity = helios.world:create()
                entity:add("position", { x = 1, y = 2 })
                entity:add("velocity", { x = 3, y = 4 })

                local position = entity:get("position")
                position.x = position.x + 9

                ecs_check = {
                    has_position = entity:has("position"),
                    count = 0,
                    before_each = entity:get("position").x
                }

                helios.world:each({ "position", "velocity" }, function(iterated_entity, iterated_position, iterated_velocity)
                    ecs_check.count = ecs_check.count + 1
                    iterated_position.x = iterated_position.x + iterated_velocity.x
                    iterated_position.y = iterated_position.y + iterated_velocity.y
                end)

                ecs_check.after_each_x = entity:get("position").x
                ecs_check.after_each_y = entity:get("position").y
                ecs_check.removed_velocity = entity:remove("velocity")
                ecs_check.has_velocity = entity:has("velocity")

                entity:destroy()
                ecs_check.valid_after_destroy = entity:is_valid()
            )",
            "ecs_smoke"
        )
    );

    sol::table ecs_check = scripts.lua()["ecs_check"];
    assert(tableBool(ecs_check, "has_position"));
    assert(tableNumber(ecs_check, "count") == 1.0);
    assert(tableNumber(ecs_check, "before_each") == 10.0);
    assert(tableNumber(ecs_check, "after_each_x") == 13.0);
    assert(tableNumber(ecs_check, "after_each_y") == 6.0);
    assert(tableBool(ecs_check, "removed_velocity"));
    assert(!tableBool(ecs_check, "has_velocity"));
    assert(!tableBool(ecs_check, "valid_after_destroy"));

    assert(
        scripts.runString(
            R"(
                helios.layers.push({
                    update = function(self, dt)
                        error("expected smoke-test failure")
                    end
                })
            )",
            "runtime_error_smoke"
        )
    );

    layer_stack.update(0.1f);
    layer_stack.clear();

    helios::ecs::World game_script_world;
    helios::layer::LayerStack game_script_layer_stack(game_script_world);
    helios::scripting::ScriptEngine game_scripts(game_script_world, game_script_layer_stack);
    game::native::registerBallPhysicsSystem(game_scripts);

    const std::filesystem::path game_script_root = std::filesystem::path(HELIOS_SOURCE_DIR)
        / "Source"
        / "Game"
        / "Scripts";

    assert(game_scripts.loadStartupScript(game_script_root, "main.lua"));

    assert(
        game_scripts.runString(
            R"(
                physics_ball = helios.world:create()
                physics_ball:add("position", { x = 100, y = 200 })
                physics_ball:add("velocity", { x = 4, y = 6 })
                physics_ball:add("acceleration", { x = 0, y = 2 })
                physics_ball:add("radius", { value = 5 })
                physics_ball:add("color", { r = 255, g = 255, b = 255, a = 255 })
            )",
            "native_physics_setup"
        )
    );

    game_script_layer_stack.update(0.5f);

    assert(
        game_scripts.runString(
            R"(
                local position = physics_ball:get("position")
                local velocity = physics_ball:get("velocity")

                physics_check = {
                    x = position.x,
                    y = position.y,
                    velocity_y = velocity.y
                }
            )",
            "native_physics_check"
        )
    );

    sol::table physics_check = game_scripts.lua()["physics_check"];
    assert(std::abs(tableNumber(physics_check, "x") - 102.0) < 0.001);
    assert(std::abs(tableNumber(physics_check, "y") - 203.5) < 0.001);
    assert(std::abs(tableNumber(physics_check, "velocity_y") - 7.0) < 0.001);

    assert(
        game_scripts.runString(
            R"(
                local ball = helios.world:create()
                ball:add("position", { x = 0, y = 0 })
                ball:add("velocity", { x = 0, y = 0 })
                ball:add("acceleration", { x = 0, y = 0 })
                ball:add("radius", { value = 32 })
                ball:add("color", { r = 255, g = 255, b = 255, a = 255 })
            )",
            "game_mouse_setup"
        )
    );

    helios::event::MouseEvent mouse_pressed = helios::event::MouseEvent::buttonPressed(0);
    game_script_layer_stack.onMouseEvent(mouse_pressed);

    helios::event::MouseEvent mouse_moved = helios::event::MouseEvent::moved(4.0f, 5.0f, 4.0f, 5.0f);
    game_script_layer_stack.onMouseEvent(mouse_moved);

    helios::event::MouseEvent mouse_released = helios::event::MouseEvent::buttonReleased(0);
    game_script_layer_stack.onMouseEvent(mouse_released);

    game_script_layer_stack.clear();

    return 0;
}
