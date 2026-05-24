#pragma once

#include <Helios/Layer/ILayer.h>

#include <sol/sol.hpp>

#include <unordered_map>

namespace helios::scripting {
    class ScriptEngine;

    class LuaLayer : public helios::layer::ILayer {
    public:
        LuaLayer(ScriptEngine& engine, sol::table layer_table);
        ~LuaLayer() override = default;

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
        void onMouseScrolledEvent(helios::event::MouseScrolledEvent& event) override;
        void onWindowClosedEvent(helios::event::WindowClosedEvent& event) override;
        void onWindowResizedEvent(helios::event::WindowResizedEvent& event) override;

    private:
        template <typename... Args>
        bool callCallback(std::string_view callback_name, Args&&... args) {
            auto callback = callbacks.find(std::string(callback_name));

            if (callback == callbacks.end()) {
                return false;
            }

            sol::protected_function_result result = callback->second(layer_table, std::forward<Args>(args)...);

            if (!result.valid()) {
                reportCallbackError(callback_name, result);
                return false;
            }

            return true;
        }

        sol::table makeEventTable(bool handled) const;
        void copyHandledFlag(sol::table event_table, helios::event::IEvent& event) const;
        void reportCallbackError(std::string_view callback_name, const sol::protected_function_result& result) const;

        ScriptEngine& engine;
        sol::table layer_table;
        std::string name;
        std::unordered_map<std::string, sol::protected_function> callbacks;
    };
}
