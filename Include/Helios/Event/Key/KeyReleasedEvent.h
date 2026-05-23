#pragma once

#include <Helios/Event/IEvent.h>

namespace helios::event {
    class KeyReleasedEvent : public IEvent {
    public:
        explicit KeyReleasedEvent(int key_code) : key_code(key_code) {}

        static EventType getStaticType() {
            return EventType::KeyReleased;
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
