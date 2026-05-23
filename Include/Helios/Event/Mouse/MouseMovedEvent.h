#pragma once

#include <Helios/Event/IEvent.h>

namespace helios::event {
    class MouseMovedEvent : public IEvent {
    public:
        MouseMovedEvent(float x, float y, float delta_x, float delta_y)
            : x(x), y(y), delta_x(delta_x), delta_y(delta_y) {}

        static EventType getStaticType() {
            return EventType::MouseMoved;
        }

        EventType getEventType() const override {
            return getStaticType();
        }

        float getX() const {
            return x;
        }

        float getY() const {
            return y;
        }

        float getDeltaX() const {
            return delta_x;
        }

        float getDeltaY() const {
            return delta_y;
        }

    private:
        float x;
        float y;
        float delta_x;
        float delta_y;
    };
}
