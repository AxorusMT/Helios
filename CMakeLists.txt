cmake_minimum_required(VERSION 3.25)

project(MyRaylibGame LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# Don't build raylib examples.
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 6.0
)

FetchContent_MakeAvailable(raylib)

FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt
    GIT_TAG v3.16.0
)

FetchContent_MakeAvailable(entt)

FetchContent_Declare(
    lua
    GIT_REPOSITORY https://github.com/lua/lua.git
    GIT_TAG v5.4.8
)

FetchContent_MakeAvailable(lua)

file(GLOB LUA_SOURCES CONFIGURE_DEPENDS
    ${lua_SOURCE_DIR}/*.c
)

list(FILTER LUA_SOURCES EXCLUDE REGEX ".*/lua\\.c$")
list(FILTER LUA_SOURCES EXCLUDE REGEX ".*/luac\\.c$")
list(FILTER LUA_SOURCES EXCLUDE REGEX ".*/ltests\\.c$")
list(FILTER LUA_SOURCES EXCLUDE REGEX ".*/onelua\\.c$")

add_library(lua_static STATIC ${LUA_SOURCES})
add_library(Lua::Lua ALIAS lua_static)

target_include_directories(lua_static
    PUBLIC
        ${lua_SOURCE_DIR}
)

target_compile_definitions(lua_static
    PRIVATE
        $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

FetchContent_Declare(
    sol2
    GIT_REPOSITORY https://github.com/ThePhD/sol2.git
    GIT_TAG v3.3.0
)

set(SOL2_TESTS OFF CACHE BOOL "" FORCE)
set(SOL2_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(sol2)

enable_testing()

# ---- Engine library ----

add_library(Helios STATIC
    Source/Helios/Application/Application.cpp
    Source/Helios/Layer/LayerStack.cpp
    Source/Helios/Scripting/LuaLayer.cpp
    Source/Helios/Scripting/ScriptEngine.cpp
    Include/Helios/Core.h
    Include/Helios/Application/Application.h
    Include/Helios/Application/ApplicationConfig.h
    Include/Helios/ECS/ECS.h
    Include/Helios/ECS/Components/AccelerationComponent.h
    Include/Helios/ECS/Components/ColorComponent.h
    Include/Helios/ECS/Components/Components.h
    Include/Helios/ECS/Components/PositionComponent.h
    Include/Helios/ECS/Components/RadiusComponent.h
    Include/Helios/ECS/Components/VelocityComponent.h
    Include/Helios/ECS/Entity.h
    Include/Helios/ECS/World.h
    Include/Helios/Layer/ILayer.h
    Include/Helios/Layer/LayerHandle.h
    Include/Helios/Layer/LayerStack.h
    Include/Helios/Scripting/LuaLayer.h
    Include/Helios/Scripting/ScriptEngine.h
    Include/Helios/Event/IEvent.h
    Include/Helios/Event/EventType.h
    Include/Helios/Event/Events.h
    Include/Helios/Event/KeyEvent.h
    Include/Helios/Event/MouseEvent.h
    Include/Helios/Event/WindowEvent.h
)

target_include_directories(Helios
    PUBLIC
        ${CMAKE_SOURCE_DIR}/Include
        ${sol2_SOURCE_DIR}/include
)

target_link_libraries(Helios PUBLIC EnTT::EnTT Lua::Lua)
target_link_libraries(Helios PRIVATE raylib)

# ---- Game executable ----

add_executable(game
    Source/Game/main.cpp
    Source/Game/Native/BallPhysicsSystem.h
    Source/Game/Native/BallPhysicsSystem.cpp
)

target_include_directories(game
    PRIVATE
        ${CMAKE_SOURCE_DIR}/Source/Game
)

target_link_libraries(game PRIVATE Helios raylib)

add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Source/Game/Scripts
        $<TARGET_FILE_DIR:game>/Scripts
)

# ---- Tests ----

add_executable(ecs_smoke_tests
    Tests/EcsSmokeTest.cpp
)

target_link_libraries(ecs_smoke_tests PRIVATE Helios)

add_test(NAME ecs_smoke_tests COMMAND ecs_smoke_tests)

add_executable(lua_smoke_tests
    Tests/LuaSmokeTest.cpp
    Source/Game/Native/BallPhysicsSystem.h
    Source/Game/Native/BallPhysicsSystem.cpp
)

target_include_directories(lua_smoke_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/Source/Game
)

target_link_libraries(lua_smoke_tests PRIVATE Helios raylib)

target_compile_definitions(lua_smoke_tests
    PRIVATE
        HELIOS_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

add_test(NAME lua_smoke_tests COMMAND lua_smoke_tests)
