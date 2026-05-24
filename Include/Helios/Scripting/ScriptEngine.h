#pragma once

#include <Helios/Core.h>
#include <Helios/ECS/ECS.h>
#include <Helios/ECS/Components/Components.h>
#include <Helios/Layer/LayerHandle.h>
#include <Helios/Layer/LayerStack.h>

#include <sol/sol.hpp>

#include <filesystem>
#include <functional>
#include <unordered_map>

namespace helios::scripting {
    class ScriptEngine {
    public:
        using NativeSystem = std::function<void(sol::variadic_args)>;

        ScriptEngine(helios::ecs::World& world, helios::layer::LayerStack& layer_stack);
        ~ScriptEngine();

        bool loadStartupScript(const std::filesystem::path& root, std::string_view startup_script);
        bool runString(std::string_view source, std::string_view chunk_name = "chunk");

        helios::layer::LayerHandle pushLuaLayer(sol::object layer_factory_or_table);
        bool removeLayer(helios::layer::LayerHandle handle);
        [[nodiscard]] bool isLayerValid(helios::layer::LayerHandle handle) const;

        void registerNativeSystem(std::string name, NativeSystem system);
        bool runNativeSystem(const std::string& name, sol::variadic_args args);

        void reportError(std::string_view context, const sol::protected_function_result& result) const;

        [[nodiscard]] sol::state& lua() {
            return lua_state;
        }

        [[nodiscard]] helios::ecs::World& getWorld() {
            return world;
        }

        [[nodiscard]] const std::filesystem::path& getScriptRoot() const {
            return script_root;
        }

    private:
        struct ComponentBinding {
            uint32_t mask_bit = 0;
            std::function<sol::object(helios::ecs::Entity&, sol::table)> add;
            std::function<sol::object(helios::ecs::Entity&, sol::table)> replace;
            std::function<sol::object(helios::ecs::Entity&)> get;
            std::function<bool(const helios::ecs::Entity&)> has;
            std::function<bool(helios::ecs::Entity&)> remove;
        };

        void registerBindings();
        void registerLayerBindings(sol::table& helios);
        void registerWorldBindings(sol::table& helios);
        void registerGraphicsBindings(sol::table& helios);
        void registerWindowBindings(sol::table& helios);
        void registerInputBindings(sol::table& helios);
        void registerRandomBindings(sol::table& helios);
        void registerLogBindings(sol::table& helios);
        void registerNativeBindings(sol::table& helios);
        void registerConstants(sol::table& helios);
        void registerComponentBindings();
        void configurePackagePath(const std::filesystem::path& root);

        sol::object addComponent(helios::ecs::Entity& entity, const std::string& component_name, sol::table values);
        sol::object replaceComponent(helios::ecs::Entity& entity, const std::string& component_name, sol::table values);
        sol::object getComponent(helios::ecs::Entity& entity, const std::string& component_name);
        bool hasComponent(const helios::ecs::Entity& entity, const std::string& component_name) const;
        bool removeComponent(helios::ecs::Entity& entity, const std::string& component_name);

        void each(sol::table component_names, sol::protected_function callback);
        sol::object getComponentObject(helios::ecs::Entity& entity, const std::string& component_name);
        std::vector<sol::object> makeComponentObjects(
            helios::ecs::Entity& entity,
            const std::vector<std::string>& component_names
        );
        void callEachCallback(
            sol::protected_function& callback,
            helios::ecs::Entity entity,
            const std::vector<sol::object>& component_objects
        );

        template <typename... TComponents>
        void eachTyped(const std::vector<std::string>& component_names, sol::protected_function callback) {
            world.each<TComponents...>(
                [this, &component_names, &callback](helios::ecs::Entity entity, TComponents&...) {
                    callEachCallback(callback, entity, makeComponentObjects(entity, component_names));
                }
            );
        }

        sol::state lua_state;
        helios::ecs::World& world;
        helios::layer::LayerStack& layer_stack;
        std::filesystem::path script_root;
        std::unordered_map<std::string, NativeSystem> native_systems;
        std::unordered_map<std::string, ComponentBinding> component_bindings;
    };
}
