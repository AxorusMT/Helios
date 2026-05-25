#pragma once

namespace helios::application {
    inline bool quit_requested = false;

    inline void requestQuit() {
        quit_requested = true;
    }

    inline bool isQuitRequested() {
        return quit_requested;
    }

    inline void clearQuitRequest() {
        quit_requested = false;
    }
}
