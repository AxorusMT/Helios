#pragma once

#include <Helios/Event/IEvent.h>

namespace helios::event {
    enum class MouseEventAction {
        ButtonPressed,
        ButtonReleased,
        Moved,
        Scrolled
    };

    constexpr std::string_view toString(const MouseEventAction action) {
        switch (action) {
            case MouseEventAction::ButtonPressed:
                return "button_pressed";
            case MouseEventAction::ButtonReleased:
                return "button_released";
            case MouseEventAction::Moved:
                return "moved";
            case MouseEventAction::Scrolled:
                return "scrolled";
        }

        return "<unknown>";
    }

    class MouseEvent : public IEvent {
    public:
        static MouseEvent buttonPressed(int button) {
            return MouseEvent(MouseEventAction::ButtonPressed, button);
        }

        static MouseEvent buttonReleased(int button) {
            return MouseEvent(MouseEventAction::ButtonReleased, button);
        }

        static MouseEvent moved(float x, float y, float delta_x, float delta_y) {
            MouseEvent event(MouseEventAction::Moved);
            event.x = x;
            event.y = y;
            event.delta_x = delta_x;
            event.delta_y = delta_y;
            return event;
        }

        static MouseEvent scrolled(float offset_x, float offset_y) {
            MouseEvent event(MouseEventAction::Scrolled);
            event.offset_x = offset_x;
            event.offset_y = offset_y;
            return event;
        }

        explicit MouseEvent(MouseEventAction action, int button = 0)
            : action(action), button(button) {}

        static EventType getStaticType() {
            return EventType::Mouse;
        }

        EventType getEventType() const override {
            return getStaticType();
        }

        MouseEventAction getAction() const {
            return action;
        }

        std::string_view getActionName() const {
            return toString(action);
        }

        bool isButtonPressed() const {
            return action == MouseEventAction::ButtonPressed;
        }

        bool isButtonReleased() const {
            return action == MouseEventAction::ButtonReleased;
        }

        bool isMoved() const {
            return action == MouseEventAction::Moved;
        }

        bool isScrolled() const {
            return action == MouseEventAction::Scrolled;
        }

        int getButton() const {
            return button;
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

        float getOffsetX() const {
            return offset_x;
        }

        float getOffsetY() const {
            return offset_y;
        }

    private:
        MouseEventAction action;
        int button = 0;
        float x = 0.0f;
        float y = 0.0f;
        float delta_x = 0.0f;
        float delta_y = 0.0f;
        float offset_x = 0.0f;
        float offset_y = 0.0f;
    };
}
