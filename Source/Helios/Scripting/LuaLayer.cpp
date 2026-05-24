#include <Helios/Scripting/LuaLayer.h>

#include <Helios/Scripting/ScriptEngine.h>

namespace {
    constexpr std::string_view callback_names[] = {
        "on_attach",
        "on_detach",
        "update",
        "draw",
        "on_key_held",
        "on_key_pressed",
        "on_key_released",
        "on_mouse_button_pressed",
        "on_mouse_button_released",
        "on_mouse_moved",
        "on_mouse_scrolled",
        "on_window_closed",
        "on_window_resized"
    };
}

helios::scripting::LuaLayer::LuaLayer(ScriptEngine& engine, sol::table layer_table)
    : engine(engine), layer_table(std::move(layer_table)) {
    name = this->layer_table.get_or("name", std::string("LuaLayer"));

    for (std::string_view callback_name : callback_names) {
        sol::object callback = this->layer_table[std::string(callback_name)];

        if (callback.get_type() == sol::type::function) {
            callbacks.emplace(std::string(callback_name), callback.as<sol::protected_function>());
        }
    }
}

void helios::scripting::LuaLayer::onAttach() {
    callCallback("on_attach");
}

void helios::scripting::LuaLayer::onDetach() {
    callCallback("on_detach");
}

void helios::scripting::LuaLayer::update(float dt) {
    callCallback("update", dt);
}

void helios::scripting::LuaLayer::draw() {
    callCallback("draw");
}

void helios::scripting::LuaLayer::onKeyHeldEvent(helios::event::KeyHeldEvent& event) {
    sol::table event_table = makeEventTable(event.handled);
    event_table["key"] = event.getKeyCode();
    callCallback("on_key_held", event_table);
    copyHandledFlag(event_table, event);
}

void helios::scripting::LuaLayer::onKeyPressedEvent(helios::event::KeyPressedEvent& event) {
    sol::table event_table = makeEventTable(event.handled);
    event_table["key"] = event.getKeyCode();
    event_table["repeat"] = event.isRepeat();
    callCallback("on_key_pressed", event_table);
    copyHandledFlag(event_table, event);
}

void helios::scripting::LuaLayer::onKeyReleasedEvent(helios::event::KeyReleasedEvent& event) {
    sol::table event_table = makeEventTable(event.handled);
    event_table["key"] = event.getKeyCode();
    callCallback("on_key_released", event_table);
    copyHandledFlag(event_table, event);
}

void helios::scripting::LuaLayer::onMouseButtonPressedEvent(helios::event::MouseButtonPressedEvent& event) {
    sol::table event_table = makeEventTable(event.handled);
    event_table["button"] = event.getButton();
    callCallback("on_mouse_button_pressed", event_table);
    copyHandledFlag(event_table, event);
}

void helios::scripting::LuaLayer::onMouseButtonReleasedEvent(helios::event::MouseButtonReleasedEvent& event) {
    sol::table event_table = makeEventTable(event.handled);
    event_table["button"] = event.getButton();
    callCallback("on_mouse_button_released", event_table);
    copyHandledFlag(event_table, event);
}

void helios::scripting::LuaLayer::onMouseMovedEvent(helios::event::MouseMovedEvent& event) {
    sol::table event_table = makeEventTable(event.handled);
    event_table["x"] = event.getX();
    event_table["y"] = event.getY();
    event_table["delta_x"] = event.getDeltaX();
    event_table["delta_y"] = event.getDeltaY();
    callCallback("on_mouse_moved", event_table);
    copyHandledFlag(event_table, event);
}

void helios::scripting::LuaLayer::onMouseScrolledEvent(helios::event::MouseScrolledEvent& event) {
    sol::table event_table = makeEventTable(event.handled);
    event_table["offset_x"] = event.getOffsetX();
    event_table["offset_y"] = event.getOffsetY();
    callCallback("on_mouse_scrolled", event_table);
    copyHandledFlag(event_table, event);
}

void helios::scripting::LuaLayer::onWindowClosedEvent(helios::event::WindowClosedEvent& event) {
    sol::table event_table = makeEventTable(event.handled);
    callCallback("on_window_closed", event_table);
    copyHandledFlag(event_table, event);
}

void helios::scripting::LuaLayer::onWindowResizedEvent(helios::event::WindowResizedEvent& event) {
    sol::table event_table = makeEventTable(event.handled);
    event_table["width"] = event.getWidth();
    event_table["height"] = event.getHeight();
    callCallback("on_window_resized", event_table);
    copyHandledFlag(event_table, event);
}

sol::table helios::scripting::LuaLayer::makeEventTable(bool handled) const {
    sol::table event_table = engine.lua().create_table();
    event_table["handled"] = handled;
    return event_table;
}

void helios::scripting::LuaLayer::copyHandledFlag(sol::table event_table, helios::event::IEvent& event) const {
    event.handled = event_table.get_or("handled", event.handled);
}

void helios::scripting::LuaLayer::reportCallbackError(
    std::string_view callback_name,
    const sol::protected_function_result& result
) const {
    std::string context = name + "." + std::string(callback_name);
    engine.reportError(context, result);
}
