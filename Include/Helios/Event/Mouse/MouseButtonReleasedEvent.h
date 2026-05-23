#pragma once

#include <Helios/Event/IEvent.h>

namespace helios::event {
    class MouseButtonReleasedEvent : public IEvent {
    public:
        explicit MouseButtonReleasedEvent(int button) : button(button) {}

        static EventType getStaticType() {
            return EventType::MouseButtonReleased;
        }

        EventType getEventType() const override {
            return getStaticType();
        }

        int getButton() const {
            return button;
        }

    private:
        int button;
    };
}
