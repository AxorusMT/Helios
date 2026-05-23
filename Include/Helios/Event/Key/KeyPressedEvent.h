#pragma once

#include <Helios/Event/IEvent.h>

namespace helios::event {
    class KeyPressedEvent : public IEvent {
    public:
        explicit KeyPressedEvent(int key_code, bool repeat = false)
            : key_code(key_code), repeat(repeat) {}

        static EventType getStaticType() {
            return EventType::KeyPressed;
        }

        EventType getEventType() const override {
            return getStaticType();
        }

        int getKeyCode() const {
            return key_code;
        }

        bool isRepeat() const {
            return repeat;
        }

    private:
        int key_code;
        bool repeat;
    };
}
