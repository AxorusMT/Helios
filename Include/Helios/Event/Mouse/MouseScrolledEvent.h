#pragma once

#include <Helios/Event/IEvent.h>

namespace helios::event {
    class MouseScrolledEvent : public IEvent {
    public:
        MouseScrolledEvent(float offset_x, float offset_y) : offset_x(offset_x), offset_y(offset_y) {}

        static EventType getStaticType() {
            return EventType::MouseScrolled;
        }

        EventType getEventType() const override {
            return getStaticType();
        }

        float getOffsetX() const {
            return offset_x;
        }

        float getOffsetY() const {
            return offset_y;
        }

    private:
        float offset_x;
        float offset_y;
    };
}
