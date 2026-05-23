#pragma once

#include <Helios/Core.h>
#include <Helios/Event/IEvent.h>

#include <functional>
#include <queue>
#include <type_traits>

namespace helios::event {
    class EventManager {
    public:
        template <typename TEvent, typename... Args>
        void pushEvent(Args&&... args) {
            static_assert(std::is_base_of_v<IEvent, TEvent>, "TEvent must implement IEvent");

            event_queue.push(std::make_unique<TEvent>(std::forward<Args>(args)...));
        }

        void pushEvent(std::unique_ptr<IEvent> event) {
            if (event == nullptr) {
                return;
            }

            event_queue.push(std::move(event));
        }

        bool pollEvent(std::unique_ptr<IEvent>& event) {
            event.reset();

            if (event_queue.empty()) {
                return false;
            }

            event = std::move(event_queue.front());
            event_queue.pop();

            return true;
        }

        bool hasEvents() const {
            return !event_queue.empty();
        }

        void clear() {
            while (!event_queue.empty()) {
                event_queue.pop();
            }
        }

        template <typename TEvent, typename TCallback>
        static bool dispatch(IEvent& event, TCallback&& callback) {
            if (event.handled || event.getEventType() != TEvent::getStaticType()) {
                return false;
            }

            event.handled = std::invoke(std::forward<TCallback>(callback), static_cast<TEvent&>(event));

            return event.handled;
        }

    private:
        std::queue<std::unique_ptr<IEvent>> event_queue;
    };
}
