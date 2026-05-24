#include <Helios/Scripting/ScriptEngine.h>

#include <Helios/Scripting/LuaLayer.h>

#include <raylib.h>

#include <algorithm>
#include <cmath>
#include <sstream>
#include <unordered_set>

namespace {
    using helios::ecs::components::AccelerationComponent;
    using helios::ecs::components::ColorComponent;
    using helios::ecs::components::PositionComponent;
    using helios::ecs::components::RadiusComponent;
    using helios::ecs::components::VelocityComponent;

    constexpr uint32_t position_bit = 1u << 0u;
    constexpr uint32_t velocity_bit = 1u << 1u;
    constexpr uint32_t acceleration_bit = 1u << 2u;
    constexpr uint32_t radius_bit = 1u << 3u;
    constexpr uint32_t color_bit = 1u << 4u;

    uint8_t toByte(int value) {
        return static_cast<uint8_t>(std::clamp(value, 0, 255));
    }

    Color colorFromObject(sol::object object, Color fallback = WHITE) {
        if (!object.valid() || object.get_type() != sol::type::table) {
            return fallback;
        }

        sol::table table = object.as<sol::table>();

        return Color{
            toByte(table.get_or("r", static_cast<int>(fallback.r))),
            toByte(table.get_or("g", static_cast<int>(fallback.g))),
            toByte(table.get_or("b", static_cast<int>(fallback.b))),
            toByte(table.get_or("a", static_cast<int>(fallback.a)))
        };
    }

    sol::table colorToTable(sol::state& lua, Color color) {
        sol::table table = lua.create_table();
        table["r"] = color.r;
        table["g"] = color.g;
        table["b"] = color.b;
        table["a"] = color.a;
        return table;
    }

    std::string valueToString(sol::object value) {
        switch (value.get_type()) {
            case sol::type::string:
                return value.as<std::string>();
            case sol::type::number: {
                std::ostringstream stream;
                stream << value.as<double>();
                return stream.str();
            }
            case sol::type::boolean:
                return value.as<bool>() ? "true" : "false";
            case sol::type::nil:
                return "nil";
            default:
                return "<" + std::string(sol::type_name(value.lua_state(), value.get_type())) + ">";
        }
    }

    std::string joinArguments(sol::variadic_args args) {
        std::ostringstream stream;
        bool first = true;

        for (sol::object value : args) {
            if (!first) {
                stream << '\t';
            }

            first = false;
            stream << valueToString(value);
        }

        return stream.str();
    }

    float tableFloat(sol::table table, const char* key, float fallback = 0.0f) {
        return table.get_or(key, fallback);
    }

    int tableInt(sol::table table, const char* key, int fallback = 0) {
        return table.get_or(key, fallback);
    }

    ColorComponent tableColorComponent(sol::table table) {
        return ColorComponent{
            toByte(tableInt(table, "r", 255)),
            toByte(tableInt(table, "g", 255)),
            toByte(tableInt(table, "b", 255)),
            toByte(tableInt(table, "a", 255))
        };
    }
}

helios::scripting::ScriptEngine::ScriptEngine(
    helios::ecs::World& world,
    helios::layer::LayerStack& layer_stack
) : world(world), layer_stack(layer_stack) {
    lua_state.open_libraries(
        sol::lib::base,
        sol::lib::package,
        sol::lib::math,
        sol::lib::table,
        sol::lib::string,
        sol::lib::utf8
    );

    registerBindings();
}

helios::scripting::ScriptEngine::~ScriptEngine() = default;

bool helios::scripting::ScriptEngine::loadStartupScript(
    const std::filesystem::path& root,
    std::string_view startup_script
) {
    script_root = std::filesystem::absolute(root);
    configurePackagePath(script_root);

    const std::filesystem::path startup_path = script_root / std::filesystem::path(startup_script);

    if (!std::filesystem::exists(startup_path)) {
        std::println("Lua startup script not found: {}", startup_path.string());
        return false;
    }

    sol::protected_function_result result = lua_state.safe_script_file(
        startup_path.string(),
        sol::script_pass_on_error
    );

    if (!result.valid()) {
        reportError(startup_path.string(), result);
        return false;
    }

    return true;
}

bool helios::scripting::ScriptEngine::runString(std::string_view source, std::string_view chunk_name) {
    sol::protected_function_result result = lua_state.safe_script(
        source,
        sol::script_pass_on_error,
        std::string(chunk_name)
    );

    if (!result.valid()) {
        reportError(chunk_name, result);
        return false;
    }

    return true;
}

helios::layer::LayerHandle helios::scripting::ScriptEngine::pushLuaLayer(sol::object layer_factory_or_table) {
    sol::table layer_table;

    if (layer_factory_or_table.get_type() == sol::type::table) {
        layer_table = layer_factory_or_table.as<sol::table>();
    } else if (layer_factory_or_table.get_type() == sol::type::function) {
        sol::protected_function factory = layer_factory_or_table.as<sol::protected_function>();
        sol::protected_function_result result = factory();

        if (!result.valid()) {
            reportError("helios.layers.push(factory)", result);
            return {};
        }

        sol::object created_layer = result.get<sol::object>();

        if (created_layer.get_type() != sol::type::table) {
            std::println("Lua layer factory must return a table");
            return {};
        }

        layer_table = created_layer.as<sol::table>();
    } else {
        std::println("helios.layers.push expects a layer table or factory function");
        return {};
    }

    auto lua_layer = std::make_unique<LuaLayer>(*this, layer_table);
    helios::layer::ILayer& pushed_layer = layer_stack.pushLayer(std::move(lua_layer));
    return pushed_layer.getLayerHandle();
}

bool helios::scripting::ScriptEngine::removeLayer(helios::layer::LayerHandle handle) {
    return layer_stack.removeLayer(handle);
}

bool helios::scripting::ScriptEngine::isLayerValid(helios::layer::LayerHandle handle) const {
    return layer_stack.containsLayer(handle);
}

void helios::scripting::ScriptEngine::registerNativeSystem(std::string name, NativeSystem system) {
    native_systems.insert_or_assign(std::move(name), std::move(system));
}

bool helios::scripting::ScriptEngine::runNativeSystem(const std::string& name, sol::variadic_args args) {
    const auto system = native_systems.find(name);

    if (system == native_systems.end()) {
        std::println("Unknown native system: {}", name);
        return false;
    }

    system->second(args);
    return true;
}

void helios::scripting::ScriptEngine::reportError(
    std::string_view context,
    const sol::protected_function_result& result
) const {
    sol::error error = result;
    std::println("Lua error in {}: {}", context, error.what());
}

void helios::scripting::ScriptEngine::registerBindings() {
    lua_state.new_usertype<helios::layer::LayerHandle>(
        "LayerHandle",
        sol::no_constructor,
        "is_valid",
        &helios::layer::LayerHandle::isValid,
        "id",
        &helios::layer::LayerHandle::getId,
        sol::meta_function::equal_to,
        [](helios::layer::LayerHandle lhs, helios::layer::LayerHandle rhs) {
            return lhs == rhs;
        }
    );

    lua_state.new_usertype<PositionComponent>(
        "PositionComponent",
        sol::no_constructor,
        "x",
        &PositionComponent::x,
        "y",
        &PositionComponent::y
    );

    lua_state.new_usertype<VelocityComponent>(
        "VelocityComponent",
        sol::no_constructor,
        "x",
        &VelocityComponent::x,
        "y",
        &VelocityComponent::y
    );

    lua_state.new_usertype<AccelerationComponent>(
        "AccelerationComponent",
        sol::no_constructor,
        "x",
        &AccelerationComponent::x,
        "y",
        &AccelerationComponent::y
    );

    lua_state.new_usertype<RadiusComponent>(
        "RadiusComponent",
        sol::no_constructor,
        "value",
        &RadiusComponent::value
    );

    lua_state.new_usertype<ColorComponent>(
        "ColorComponent",
        sol::no_constructor,
        "r",
        &ColorComponent::r,
        "g",
        &ColorComponent::g,
        "b",
        &ColorComponent::b,
        "a",
        &ColorComponent::a
    );

    sol::usertype<helios::ecs::Entity> entity_type = lua_state.new_usertype<helios::ecs::Entity>(
        "Entity",
        sol::no_constructor
    );

    entity_type.set_function("is_valid", &helios::ecs::Entity::isValid);
    entity_type.set_function(
        "destroy",
        [](helios::ecs::Entity& entity) {
            if (entity.isValid()) {
                entity.destroy();
            }
        }
    );
    entity_type.set_function(
        "add",
        [this](helios::ecs::Entity& entity, const std::string& component_name, sol::table values) {
            return addComponent(entity, component_name, values);
        }
    );
    entity_type.set_function(
        "replace",
        [this](helios::ecs::Entity& entity, const std::string& component_name, sol::table values) {
            return replaceComponent(entity, component_name, values);
        }
    );
    entity_type.set_function(
        "get",
        [this](helios::ecs::Entity& entity, const std::string& component_name) {
            return getComponent(entity, component_name);
        }
    );
    entity_type.set_function(
        "has",
        [this](const helios::ecs::Entity& entity, const std::string& component_name) {
            return hasComponent(entity, component_name);
        }
    );
    entity_type.set_function(
        "remove",
        [this](helios::ecs::Entity& entity, const std::string& component_name) {
            return removeComponent(entity, component_name);
        }
    );

    registerComponentBindings();

    sol::table helios = lua_state.create_named_table("helios");
    registerLayerBindings(helios);
    registerWorldBindings(helios);
    registerGraphicsBindings(helios);
    registerWindowBindings(helios);
    registerInputBindings(helios);
    registerRandomBindings(helios);
    registerLogBindings(helios);
    registerNativeBindings(helios);
    registerConstants(helios);
}

void helios::scripting::ScriptEngine::registerLayerBindings(sol::table& helios) {
    sol::table layers = lua_state.create_table();

    layers.set_function(
        "push",
        [this](sol::object layer_factory_or_table) {
            return pushLuaLayer(layer_factory_or_table);
        }
    );
    layers.set_function(
        "remove",
        [this](helios::layer::LayerHandle handle) {
            return removeLayer(handle);
        }
    );
    layers.set_function(
        "pop",
        [this]() {
            layer_stack.popLayer();
        }
    );
    layers.set_function(
        "is_valid",
        [this](helios::layer::LayerHandle handle) {
            return isLayerValid(handle);
        }
    );

    helios["layers"] = layers;
}

void helios::scripting::ScriptEngine::registerWorldBindings(sol::table& helios) {
    sol::table world_table = lua_state.create_table();

    world_table.set_function(
        "create",
        [this](sol::table) {
            return world.createEntity();
        }
    );
    world_table.set_function(
        "each",
        [this](sol::table, sol::table component_names, sol::protected_function callback) {
            each(component_names, std::move(callback));
        }
    );

    helios["world"] = world_table;
}

void helios::scripting::ScriptEngine::registerGraphicsBindings(sol::table& helios) {
    sol::table graphics = lua_state.create_table();

    graphics.set_function(
        "rectangle",
        [](float x, float y, float width, float height, sol::object color) {
            DrawRectangle(
                static_cast<int>(std::round(x)),
                static_cast<int>(std::round(y)),
                static_cast<int>(std::round(width)),
                static_cast<int>(std::round(height)),
                colorFromObject(color)
            );
        }
    );
    graphics.set_function(
        "rectangle_gradient_v",
        [](float x, float y, float width, float height, sol::object top_color, sol::object bottom_color) {
            DrawRectangleGradientV(
                static_cast<int>(std::round(x)),
                static_cast<int>(std::round(y)),
                static_cast<int>(std::round(width)),
                static_cast<int>(std::round(height)),
                colorFromObject(top_color),
                colorFromObject(bottom_color)
            );
        }
    );
    graphics.set_function(
        "rectangle_lines",
        [](float x, float y, float width, float height, float line_thick, sol::object color) {
            DrawRectangleLinesEx(Rectangle{ x, y, width, height }, line_thick, colorFromObject(color));
        }
    );
    graphics.set_function(
        "circle",
        [](float x, float y, float radius, sol::object color) {
            DrawCircleV(Vector2{ x, y }, radius, colorFromObject(color));
        }
    );
    graphics.set_function(
        "circle_lines",
        [](float x, float y, float radius, sol::object color) {
            DrawCircleLines(
                static_cast<int>(std::round(x)),
                static_cast<int>(std::round(y)),
                radius,
                colorFromObject(color)
            );
        }
    );
    graphics.set_function(
        "triangle",
        [](float x1, float y1, float x2, float y2, float x3, float y3, sol::object color) {
            DrawTriangle(Vector2{ x1, y1 }, Vector2{ x2, y2 }, Vector2{ x3, y3 }, colorFromObject(color));
        }
    );
    graphics.set_function(
        "line",
        [](float x1, float y1, float x2, float y2, sol::object color) {
            DrawLine(
                static_cast<int>(std::round(x1)),
                static_cast<int>(std::round(y1)),
                static_cast<int>(std::round(x2)),
                static_cast<int>(std::round(y2)),
                colorFromObject(color)
            );
        }
    );
    graphics.set_function(
        "text",
        [](const std::string& text, float x, float y, int font_size, sol::object color) {
            DrawText(
                text.c_str(),
                static_cast<int>(std::round(x)),
                static_cast<int>(std::round(y)),
                font_size,
                colorFromObject(color)
            );
        }
    );
    graphics.set_function(
        "measure_text",
        [](const std::string& text, int font_size) {
            return MeasureText(text.c_str(), font_size);
        }
    );
    graphics.set_function(
        "fade",
        [this](sol::object color, float alpha) {
            return colorToTable(lua_state, Fade(colorFromObject(color), alpha));
        }
    );

    helios["graphics"] = graphics;
}

void helios::scripting::ScriptEngine::registerWindowBindings(sol::table& helios) {
    sol::table window = lua_state.create_table();

    window.set_function("width", []() {
        return GetScreenWidth();
    });
    window.set_function("height", []() {
        return GetScreenHeight();
    });

    helios["window"] = window;
}

void helios::scripting::ScriptEngine::registerInputBindings(sol::table& helios) {
    sol::table input = lua_state.create_table();

    input.set_function(
        "mouse_position",
        [this]() {
            const Vector2 position = GetMousePosition();
            sol::table table = lua_state.create_table();
            table["x"] = position.x;
            table["y"] = position.y;
            return table;
        }
    );

    helios["input"] = input;
}

void helios::scripting::ScriptEngine::registerRandomBindings(sol::table& helios) {
    sol::table random = lua_state.create_table();

    random.set_function("int", [](int min, int max) {
        return GetRandomValue(min, max);
    });

    helios["random"] = random;
}

void helios::scripting::ScriptEngine::registerLogBindings(sol::table& helios) {
    sol::table log = lua_state.create_table();

    log.set_function("info", [](sol::variadic_args args) {
        std::println("{}", joinArguments(args));
    });
    log.set_function("warn", [](sol::variadic_args args) {
        std::println("warning: {}", joinArguments(args));
    });
    log.set_function("error", [](sol::variadic_args args) {
        std::println("error: {}", joinArguments(args));
    });

    helios["log"] = log;
}

void helios::scripting::ScriptEngine::registerNativeBindings(sol::table& helios) {
    sol::table native = lua_state.create_table();

    native.set_function(
        "run",
        [this](const std::string& name, sol::variadic_args args) {
            return runNativeSystem(name, args);
        }
    );

    helios["native"] = native;
}

void helios::scripting::ScriptEngine::registerConstants(sol::table& helios) {
    sol::table key = lua_state.create_table();
    key["space"] = KEY_SPACE;
    key["up"] = KEY_UP;
    key["down"] = KEY_DOWN;
    key["left"] = KEY_LEFT;
    key["right"] = KEY_RIGHT;
    key["w"] = KEY_W;
    key["a"] = KEY_A;
    key["s"] = KEY_S;
    key["d"] = KEY_D;
    helios["key"] = key;

    sol::table mouse = lua_state.create_table();
    mouse["left"] = MOUSE_BUTTON_LEFT;
    mouse["right"] = MOUSE_BUTTON_RIGHT;
    mouse["middle"] = MOUSE_BUTTON_MIDDLE;
    helios["mouse"] = mouse;
}

void helios::scripting::ScriptEngine::registerComponentBindings() {
    component_bindings.emplace(
        "position",
        ComponentBinding{
            position_bit,
            [this](helios::ecs::Entity& entity, sol::table values) -> sol::object {
                if (!entity.isValid() || entity.hasComponent<PositionComponent>()) {
                    return sol::make_object(lua_state, sol::nil);
                }

                PositionComponent& component = entity.addComponent<PositionComponent>(
                    PositionComponent{ tableFloat(values, "x"), tableFloat(values, "y") }
                );
                return sol::make_object(lua_state, std::ref(component));
            },
            [this](helios::ecs::Entity& entity, sol::table values) -> sol::object {
                if (!entity.isValid()) {
                    return sol::make_object(lua_state, sol::nil);
                }

                PositionComponent& component = entity.addOrReplaceComponent<PositionComponent>(
                    PositionComponent{ tableFloat(values, "x"), tableFloat(values, "y") }
                );
                return sol::make_object(lua_state, std::ref(component));
            },
            [this](helios::ecs::Entity& entity) -> sol::object {
                PositionComponent* component = entity.tryGetComponent<PositionComponent>();
                return component == nullptr ? sol::make_object(lua_state, sol::nil) : sol::make_object(lua_state, std::ref(*component));
            },
            [](const helios::ecs::Entity& entity) {
                return entity.hasComponent<PositionComponent>();
            },
            [](helios::ecs::Entity& entity) {
                if (!entity.hasComponent<PositionComponent>()) return false;
                entity.removeComponent<PositionComponent>();
                return true;
            }
        }
    );

    component_bindings.emplace(
        "velocity",
        ComponentBinding{
            velocity_bit,
            [this](helios::ecs::Entity& entity, sol::table values) -> sol::object {
                if (!entity.isValid() || entity.hasComponent<VelocityComponent>()) {
                    return sol::make_object(lua_state, sol::nil);
                }

                VelocityComponent& component = entity.addComponent<VelocityComponent>(
                    VelocityComponent{ tableFloat(values, "x"), tableFloat(values, "y") }
                );
                return sol::make_object(lua_state, std::ref(component));
            },
            [this](helios::ecs::Entity& entity, sol::table values) -> sol::object {
                if (!entity.isValid()) {
                    return sol::make_object(lua_state, sol::nil);
                }

                VelocityComponent& component = entity.addOrReplaceComponent<VelocityComponent>(
                    VelocityComponent{ tableFloat(values, "x"), tableFloat(values, "y") }
                );
                return sol::make_object(lua_state, std::ref(component));
            },
            [this](helios::ecs::Entity& entity) -> sol::object {
                VelocityComponent* component = entity.tryGetComponent<VelocityComponent>();
                return component == nullptr ? sol::make_object(lua_state, sol::nil) : sol::make_object(lua_state, std::ref(*component));
            },
            [](const helios::ecs::Entity& entity) {
                return entity.hasComponent<VelocityComponent>();
            },
            [](helios::ecs::Entity& entity) {
                if (!entity.hasComponent<VelocityComponent>()) return false;
                entity.removeComponent<VelocityComponent>();
                return true;
            }
        }
    );

    component_bindings.emplace(
        "acceleration",
        ComponentBinding{
            acceleration_bit,
            [this](helios::ecs::Entity& entity, sol::table values) -> sol::object {
                if (!entity.isValid() || entity.hasComponent<AccelerationComponent>()) {
                    return sol::make_object(lua_state, sol::nil);
                }

                AccelerationComponent& component = entity.addComponent<AccelerationComponent>(
                    AccelerationComponent{ tableFloat(values, "x"), tableFloat(values, "y") }
                );
                return sol::make_object(lua_state, std::ref(component));
            },
            [this](helios::ecs::Entity& entity, sol::table values) -> sol::object {
                if (!entity.isValid()) {
                    return sol::make_object(lua_state, sol::nil);
                }

                AccelerationComponent& component = entity.addOrReplaceComponent<AccelerationComponent>(
                    AccelerationComponent{ tableFloat(values, "x"), tableFloat(values, "y") }
                );
                return sol::make_object(lua_state, std::ref(component));
            },
            [this](helios::ecs::Entity& entity) -> sol::object {
                AccelerationComponent* component = entity.tryGetComponent<AccelerationComponent>();
                return component == nullptr ? sol::make_object(lua_state, sol::nil) : sol::make_object(lua_state, std::ref(*component));
            },
            [](const helios::ecs::Entity& entity) {
                return entity.hasComponent<AccelerationComponent>();
            },
            [](helios::ecs::Entity& entity) {
                if (!entity.hasComponent<AccelerationComponent>()) return false;
                entity.removeComponent<AccelerationComponent>();
                return true;
            }
        }
    );

    component_bindings.emplace(
        "radius",
        ComponentBinding{
            radius_bit,
            [this](helios::ecs::Entity& entity, sol::table values) -> sol::object {
                if (!entity.isValid() || entity.hasComponent<RadiusComponent>()) {
                    return sol::make_object(lua_state, sol::nil);
                }

                RadiusComponent& component = entity.addComponent<RadiusComponent>(
                    RadiusComponent{ tableFloat(values, "value") }
                );
                return sol::make_object(lua_state, std::ref(component));
            },
            [this](helios::ecs::Entity& entity, sol::table values) -> sol::object {
                if (!entity.isValid()) {
                    return sol::make_object(lua_state, sol::nil);
                }

                RadiusComponent& component = entity.addOrReplaceComponent<RadiusComponent>(
                    RadiusComponent{ tableFloat(values, "value") }
                );
                return sol::make_object(lua_state, std::ref(component));
            },
            [this](helios::ecs::Entity& entity) -> sol::object {
                RadiusComponent* component = entity.tryGetComponent<RadiusComponent>();
                return component == nullptr ? sol::make_object(lua_state, sol::nil) : sol::make_object(lua_state, std::ref(*component));
            },
            [](const helios::ecs::Entity& entity) {
                return entity.hasComponent<RadiusComponent>();
            },
            [](helios::ecs::Entity& entity) {
                if (!entity.hasComponent<RadiusComponent>()) return false;
                entity.removeComponent<RadiusComponent>();
                return true;
            }
        }
    );

    component_bindings.emplace(
        "color",
        ComponentBinding{
            color_bit,
            [this](helios::ecs::Entity& entity, sol::table values) -> sol::object {
                if (!entity.isValid() || entity.hasComponent<ColorComponent>()) {
                    return sol::make_object(lua_state, sol::nil);
                }

                ColorComponent& component = entity.addComponent<ColorComponent>(tableColorComponent(values));
                return sol::make_object(lua_state, std::ref(component));
            },
            [this](helios::ecs::Entity& entity, sol::table values) -> sol::object {
                if (!entity.isValid()) {
                    return sol::make_object(lua_state, sol::nil);
                }

                ColorComponent& component = entity.addOrReplaceComponent<ColorComponent>(tableColorComponent(values));
                return sol::make_object(lua_state, std::ref(component));
            },
            [this](helios::ecs::Entity& entity) -> sol::object {
                ColorComponent* component = entity.tryGetComponent<ColorComponent>();
                return component == nullptr ? sol::make_object(lua_state, sol::nil) : sol::make_object(lua_state, std::ref(*component));
            },
            [](const helios::ecs::Entity& entity) {
                return entity.hasComponent<ColorComponent>();
            },
            [](helios::ecs::Entity& entity) {
                if (!entity.hasComponent<ColorComponent>()) return false;
                entity.removeComponent<ColorComponent>();
                return true;
            }
        }
    );
}

void helios::scripting::ScriptEngine::configurePackagePath(const std::filesystem::path& root) {
    sol::table package = lua_state["package"];
    const std::string existing_path = package.get_or("path", std::string());
    const std::string root_path = root.generic_string();

    package["path"] =
        root_path + "/?.lua;"
        + root_path + "/?/init.lua;"
        + existing_path;
}

sol::object helios::scripting::ScriptEngine::addComponent(
    helios::ecs::Entity& entity,
    const std::string& component_name,
    sol::table values
) {
    const auto binding = component_bindings.find(component_name);

    if (binding == component_bindings.end()) {
        std::println("Unknown component: {}", component_name);
        return sol::make_object(lua_state, sol::nil);
    }

    return binding->second.add(entity, values);
}

sol::object helios::scripting::ScriptEngine::replaceComponent(
    helios::ecs::Entity& entity,
    const std::string& component_name,
    sol::table values
) {
    const auto binding = component_bindings.find(component_name);

    if (binding == component_bindings.end()) {
        std::println("Unknown component: {}", component_name);
        return sol::make_object(lua_state, sol::nil);
    }

    return binding->second.replace(entity, values);
}

sol::object helios::scripting::ScriptEngine::getComponent(
    helios::ecs::Entity& entity,
    const std::string& component_name
) {
    const auto binding = component_bindings.find(component_name);

    if (binding == component_bindings.end()) {
        std::println("Unknown component: {}", component_name);
        return sol::make_object(lua_state, sol::nil);
    }

    return binding->second.get(entity);
}

bool helios::scripting::ScriptEngine::hasComponent(
    const helios::ecs::Entity& entity,
    const std::string& component_name
) const {
    const auto binding = component_bindings.find(component_name);

    if (binding == component_bindings.end()) {
        return false;
    }

    return binding->second.has(entity);
}

bool helios::scripting::ScriptEngine::removeComponent(
    helios::ecs::Entity& entity,
    const std::string& component_name
) {
    const auto binding = component_bindings.find(component_name);

    if (binding == component_bindings.end()) {
        std::println("Unknown component: {}", component_name);
        return false;
    }

    return binding->second.remove(entity);
}

void helios::scripting::ScriptEngine::each(sol::table component_names, sol::protected_function callback) {
    std::vector<std::string> names;
    uint32_t mask = 0;

    for (std::size_t index = 1; true; ++index) {
        sol::optional<std::string> name = component_names[index];

        if (!name.has_value()) {
            break;
        }

        const auto binding = component_bindings.find(name.value());

        if (binding == component_bindings.end()) {
            std::println("Unknown component in helios.world:each: {}", name.value());
            return;
        }

        if ((mask & binding->second.mask_bit) != 0u) {
            std::println("Duplicate component in helios.world:each: {}", name.value());
            return;
        }

        mask |= binding->second.mask_bit;
        names.push_back(name.value());
    }

    if (names.empty()) {
        std::println("helios.world:each requires at least one component");
        return;
    }

    switch (mask) {
        case position_bit:
            eachTyped<PositionComponent>(names, std::move(callback));
            break;
        case velocity_bit:
            eachTyped<VelocityComponent>(names, std::move(callback));
            break;
        case acceleration_bit:
            eachTyped<AccelerationComponent>(names, std::move(callback));
            break;
        case radius_bit:
            eachTyped<RadiusComponent>(names, std::move(callback));
            break;
        case color_bit:
            eachTyped<ColorComponent>(names, std::move(callback));
            break;
        case position_bit | velocity_bit:
            eachTyped<PositionComponent, VelocityComponent>(names, std::move(callback));
            break;
        case position_bit | acceleration_bit:
            eachTyped<PositionComponent, AccelerationComponent>(names, std::move(callback));
            break;
        case position_bit | radius_bit:
            eachTyped<PositionComponent, RadiusComponent>(names, std::move(callback));
            break;
        case position_bit | color_bit:
            eachTyped<PositionComponent, ColorComponent>(names, std::move(callback));
            break;
        case velocity_bit | acceleration_bit:
            eachTyped<VelocityComponent, AccelerationComponent>(names, std::move(callback));
            break;
        case velocity_bit | radius_bit:
            eachTyped<VelocityComponent, RadiusComponent>(names, std::move(callback));
            break;
        case velocity_bit | color_bit:
            eachTyped<VelocityComponent, ColorComponent>(names, std::move(callback));
            break;
        case acceleration_bit | radius_bit:
            eachTyped<AccelerationComponent, RadiusComponent>(names, std::move(callback));
            break;
        case acceleration_bit | color_bit:
            eachTyped<AccelerationComponent, ColorComponent>(names, std::move(callback));
            break;
        case radius_bit | color_bit:
            eachTyped<RadiusComponent, ColorComponent>(names, std::move(callback));
            break;
        case position_bit | velocity_bit | acceleration_bit:
            eachTyped<PositionComponent, VelocityComponent, AccelerationComponent>(names, std::move(callback));
            break;
        case position_bit | velocity_bit | radius_bit:
            eachTyped<PositionComponent, VelocityComponent, RadiusComponent>(names, std::move(callback));
            break;
        case position_bit | velocity_bit | color_bit:
            eachTyped<PositionComponent, VelocityComponent, ColorComponent>(names, std::move(callback));
            break;
        case position_bit | acceleration_bit | radius_bit:
            eachTyped<PositionComponent, AccelerationComponent, RadiusComponent>(names, std::move(callback));
            break;
        case position_bit | acceleration_bit | color_bit:
            eachTyped<PositionComponent, AccelerationComponent, ColorComponent>(names, std::move(callback));
            break;
        case position_bit | radius_bit | color_bit:
            eachTyped<PositionComponent, RadiusComponent, ColorComponent>(names, std::move(callback));
            break;
        case velocity_bit | acceleration_bit | radius_bit:
            eachTyped<VelocityComponent, AccelerationComponent, RadiusComponent>(names, std::move(callback));
            break;
        case velocity_bit | acceleration_bit | color_bit:
            eachTyped<VelocityComponent, AccelerationComponent, ColorComponent>(names, std::move(callback));
            break;
        case velocity_bit | radius_bit | color_bit:
            eachTyped<VelocityComponent, RadiusComponent, ColorComponent>(names, std::move(callback));
            break;
        case acceleration_bit | radius_bit | color_bit:
            eachTyped<AccelerationComponent, RadiusComponent, ColorComponent>(names, std::move(callback));
            break;
        case position_bit | velocity_bit | acceleration_bit | radius_bit:
            eachTyped<PositionComponent, VelocityComponent, AccelerationComponent, RadiusComponent>(names, std::move(callback));
            break;
        case position_bit | velocity_bit | acceleration_bit | color_bit:
            eachTyped<PositionComponent, VelocityComponent, AccelerationComponent, ColorComponent>(names, std::move(callback));
            break;
        case position_bit | velocity_bit | radius_bit | color_bit:
            eachTyped<PositionComponent, VelocityComponent, RadiusComponent, ColorComponent>(names, std::move(callback));
            break;
        case position_bit | acceleration_bit | radius_bit | color_bit:
            eachTyped<PositionComponent, AccelerationComponent, RadiusComponent, ColorComponent>(names, std::move(callback));
            break;
        case velocity_bit | acceleration_bit | radius_bit | color_bit:
            eachTyped<VelocityComponent, AccelerationComponent, RadiusComponent, ColorComponent>(names, std::move(callback));
            break;
        case position_bit | velocity_bit | acceleration_bit | radius_bit | color_bit:
            eachTyped<PositionComponent, VelocityComponent, AccelerationComponent, RadiusComponent, ColorComponent>(
                names,
                std::move(callback)
            );
            break;
        default:
            std::println("Unsupported component query mask: {}", mask);
            break;
    }
}

sol::object helios::scripting::ScriptEngine::getComponentObject(
    helios::ecs::Entity& entity,
    const std::string& component_name
) {
    const auto binding = component_bindings.find(component_name);

    if (binding == component_bindings.end()) {
        return sol::make_object(lua_state, sol::nil);
    }

    return binding->second.get(entity);
}

std::vector<sol::object> helios::scripting::ScriptEngine::makeComponentObjects(
    helios::ecs::Entity& entity,
    const std::vector<std::string>& component_names
) {
    std::vector<sol::object> component_objects;
    component_objects.reserve(component_names.size());

    for (const std::string& component_name : component_names) {
        component_objects.push_back(getComponentObject(entity, component_name));
    }

    return component_objects;
}

void helios::scripting::ScriptEngine::callEachCallback(
    sol::protected_function& callback,
    helios::ecs::Entity entity,
    const std::vector<sol::object>& component_objects
) {
    sol::object entity_object = sol::make_object(lua_state, helios::ecs::Entity(entity));

    sol::protected_function_result result = [&]() {
        switch (component_objects.size()) {
            case 1:
                return callback(entity_object, component_objects[0]);
            case 2:
                return callback(entity_object, component_objects[0], component_objects[1]);
            case 3:
                return callback(entity_object, component_objects[0], component_objects[1], component_objects[2]);
            case 4:
                return callback(
                    entity_object,
                    component_objects[0],
                    component_objects[1],
                    component_objects[2],
                    component_objects[3]
                );
            case 5:
                return callback(
                    entity_object,
                    component_objects[0],
                    component_objects[1],
                    component_objects[2],
                    component_objects[3],
                    component_objects[4]
                );
            default:
                return callback(entity_object);
        }
    }();

    if (!result.valid()) {
        reportError("helios.world:each", result);
    }
}
