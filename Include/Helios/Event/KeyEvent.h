#pragma once

#include <Helios/Core.h>
#include <Helios/Event/IEvent.h>

namespace helios::event {
    class KeyEvent : public IEvent {
    public:
        int getKeyCode() const {
            return key_code;
        }

    protected:
        explicit KeyEvent(int key_code) : key_code(key_code) {}

    private:
        int key_code;
    };

    class KeyPressed : public KeyEvent {
    public:
        explicit KeyPressed(int key_code, bool repeat = false)
            : KeyEvent(key_code), repeat(repeat) {}

        EventType getEventType() const override {
            return EventType::KeyPressed;
        }

        bool isRepeat() const {
            return repeat;
        }

    private:
        bool repeat;
    };

    class KeyReleased : public KeyEvent {
    public:
        explicit KeyReleased(int key_code) : KeyEvent(key_code) {}

        EventType getEventType() const override {
            return EventType::KeyReleased;
        }
    };
}
