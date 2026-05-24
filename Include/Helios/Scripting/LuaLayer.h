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
        void onKeyEvent(helios::event::KeyEvent& event) override;
        void onMouseEvent(helios::event::MouseEvent& event) override;
        void onWindowEvent(helios::event::WindowEvent& event) override;

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
