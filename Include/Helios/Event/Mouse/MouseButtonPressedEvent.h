#pragma once

#include <Helios/Event/IEvent.h>

namespace helios::event {
    class MouseButtonPressedEvent : public IEvent {
    public:
        explicit MouseButtonPressedEvent(int button) : button(button) {}

        static EventType getStaticType() {
            return EventType::MouseButtonPressed;
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
