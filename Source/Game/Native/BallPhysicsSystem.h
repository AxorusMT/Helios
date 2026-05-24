#pragma once

namespace helios::scripting {
    class ScriptEngine;
}

namespace game::native {
    void registerBallPhysicsSystem(helios::scripting::ScriptEngine& scripts);
}
