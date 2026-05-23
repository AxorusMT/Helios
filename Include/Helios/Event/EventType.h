#pragma once

// Here, we use an x-macro to get some simple reflection

#include <Helios/Core.h>

namespace helios::event {

    #define HELIOS_ENGINE_EVENT_TYPE_XMACRO_LIST(X) \
        X(None) \
        X(KeyPressed) \
        X(KeyReleased) 

    enum class EventType {
        #define HELIOS_ENGINE_EVENT_TYPE_DECLARE(name) name,
            HELIOS_ENGINE_EVENT_TYPE_XMACRO_LIST(HELIOS_ENGINE_EVENT_TYPE_DECLARE) 
        #undef HELIOS_ENGINE_EVENT_TYPE_DECLARE
    };

    constexpr std::string_view toString(const EventType type) {
        switch (type) {
            #define HELIOS_ENGINE_EVENT_TYPE_TO_STRING(name) \
                case EventType::name: \
                    return #name; 

                HELIOS_ENGINE_EVENT_TYPE_XMACRO_LIST(HELIOS_ENGINE_EVENT_TYPE_TO_STRING)

            #undef HELIOS_ENGINE_EVENT_TYPE_TO_STRING
        }

        return "<unknown>";
    }
}