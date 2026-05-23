#pragma once

#include <Helios/Event/IEvent.h>

namespace helios::event {
    class KeyHeldEvent : public IEvent {
    public:
        explicit KeyHeldEvent(int key_code) : key_code(key_code) {}

        static EventType getStaticType() {
            return EventType::KeyHeld;
        }

        EventType getEventType() const override {
            return getStaticType();
        }

        int getKeyCode() const {
            return key_code;
        }

    private:
        int key_code;
    };
}
