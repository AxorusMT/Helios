#pragma once

#include <Helios/Event/IEvent.h>

namespace helios::event {
    enum class WindowEventAction {
        Closed,
        Resized
    };

    constexpr std::string_view toString(const WindowEventAction action) {
        switch (action) {
            case WindowEventAction::Closed:
                return "closed";
            case WindowEventAction::Resized:
                return "resized";
        }

        return "<unknown>";
    }

    class WindowEvent : public IEvent {
    public:
        static WindowEvent closed() {
            return WindowEvent(WindowEventAction::Closed);
        }

        static WindowEvent resized(int width, int height) {
            WindowEvent event(WindowEventAction::Resized);
            event.width = width;
            event.height = height;
            return event;
        }

        explicit WindowEvent(WindowEventAction action) : action(action) {}

        static EventType getStaticType() {
            return EventType::Window;
        }

        EventType getEventType() const override {
            return getStaticType();
        }

        WindowEventAction getAction() const {
            return action;
        }

        std::string_view getActionName() const {
            return toString(action);
        }

        bool isClosed() const {
            return action == WindowEventAction::Closed;
        }

        bool isResized() const {
            return action == WindowEventAction::Resized;
        }

        int getWidth() const {
            return width;
        }

        int getHeight() const {
            return height;
        }

    private:
        WindowEventAction action;
        int width = 0;
        int height = 0;
    };
}
