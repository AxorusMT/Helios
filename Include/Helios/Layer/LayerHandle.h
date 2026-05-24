#pragma once

#include <Helios/Core.h>

namespace helios::layer {
    class LayerStack;

    class LayerHandle {
    public:
        LayerHandle() = default;

        [[nodiscard]] bool isValid() const {
            return id != invalid_id;
        }

        [[nodiscard]] uint64_t getId() const {
            return id;
        }

        friend bool operator==(LayerHandle lhs, LayerHandle rhs) {
            return lhs.id == rhs.id;
        }

        friend bool operator!=(LayerHandle lhs, LayerHandle rhs) {
            return !(lhs == rhs);
        }

    private:
        friend class LayerStack;

        explicit LayerHandle(uint64_t id) : id(id) {}

        static constexpr uint64_t invalid_id = 0;

        uint64_t id = invalid_id;
    };
}
