#pragma once

#include <Helios/Core.h>
#include "EventType.h"

namespace helios::event {
    class IEvent {
    public:
        bool handled = false;

        virtual ~IEvent() = default;

        virtual EventType getEventType() const = 0;
    };
}