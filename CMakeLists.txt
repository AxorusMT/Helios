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

enable_testing()

# ---- Engine library ----

add_library(Helios STATIC
    Source/Helios/Application/Application.cpp
    Source/Helios/Layer/LayerStack.cpp
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
    Include/Helios/Layer/LayerStack.h
    Include/Helios/Event/IEvent.h
    Include/Helios/Event/EventType.h
    Include/Helios/Event/Events.h
    Include/Helios/Event/Key/KeyHeldEvent.h
    Include/Helios/Event/Key/KeyPressedEvent.h
    Include/Helios/Event/Key/KeyReleasedEvent.h
    Include/Helios/Event/Mouse/MouseButtonPressedEvent.h
    Include/Helios/Event/Mouse/MouseButtonReleasedEvent.h
    Include/Helios/Event/Mouse/MouseMovedEvent.h
    Include/Helios/Event/Mouse/MouseScrolledEvent.h
    Include/Helios/Event/Window/WindowClosedEvent.h
    Include/Helios/Event/Window/WindowResizedEvent.h
)

target_include_directories(Helios
    PUBLIC
        ${CMAKE_SOURCE_DIR}/Include
)

target_link_libraries(Helios PUBLIC EnTT::EnTT)
target_link_libraries(Helios PRIVATE raylib)

# ---- Game executable ----

add_executable(game
    Source/Game/main.cpp
    Source/Game/Layer/TestLayerA.h
    Source/Game/Layer/TestLayerA.cpp
    Source/Game/Layer/TestLayerB.h
    Source/Game/Layer/TestLayerB.cpp
)

target_include_directories(game
    PRIVATE
        ${CMAKE_SOURCE_DIR}/Source/Game
)

target_link_libraries(game PRIVATE Helios raylib)

# ---- Tests ----

add_executable(ecs_smoke_tests
    Source/Tests/EcsSmokeTest.cpp
)

target_link_libraries(ecs_smoke_tests PRIVATE Helios)

add_test(NAME ecs_smoke_tests COMMAND ecs_smoke_tests)
