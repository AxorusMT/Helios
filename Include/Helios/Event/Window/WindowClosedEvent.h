#pragma once

#include <Helios/Event/IEvent.h>

namespace helios::event {
    class WindowClosedEvent : public IEvent {
    public:
        static EventType getStaticType() {
            return EventType::WindowClosed;
        }

        EventType getEventType() const override {
            return getStaticType();
        }
    };
}
