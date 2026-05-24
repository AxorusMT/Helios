#pragma once

#include <Helios/Event/IEvent.h>

namespace helios::event {
    enum class KeyEventAction {
        Held,
        Pressed,
        Released
    };

    constexpr std::string_view toString(const KeyEventAction action) {
        switch (action) {
            case KeyEventAction::Held:
                return "held";
            case KeyEventAction::Pressed:
                return "pressed";
            case KeyEventAction::Released:
                return "released";
        }

        return "<unknown>";
    }

    class KeyEvent : public IEvent {
    public:
        KeyEvent(KeyEventAction action, int key_code, bool repeat = false)
            : action(action), key_code(key_code), repeat(repeat) {}

        static EventType getStaticType() {
            return EventType::Key;
        }

        EventType getEventType() const override {
            return getStaticType();
        }

        KeyEventAction getAction() const {
            return action;
        }

        std::string_view getActionName() const {
            return toString(action);
        }

        bool isHeld() const {
            return action == KeyEventAction::Held;
        }

        bool isPressed() const {
            return action == KeyEventAction::Pressed;
        }

        bool isReleased() const {
            return action == KeyEventAction::Released;
        }

        int getKeyCode() const {
            return key_code;
        }

        bool isRepeat() const {
            return repeat;
        }

    private:
        KeyEventAction action;
        int key_code;
        bool repeat;
    };
}
