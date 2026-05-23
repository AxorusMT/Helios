#pragma once

#include <Helios/Event/IEvent.h>

namespace helios::event {
    class WindowResizedEvent : public IEvent {
    public:
        WindowResizedEvent(int width, int height) : width(width), height(height) {}

        static EventType getStaticType() {
            return EventType::WindowResized;
        }

        EventType getEventType() const override {
            return getStaticType();
        }

        int getWidth() const {
            return width;
        }

        int getHeight() const {
            return height;
        }

    private:
        int width;
        int height;
    };
}
