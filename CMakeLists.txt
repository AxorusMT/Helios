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

# ---- Engine library ----

add_library(Helios STATIC
    Source/Helios/Helios.cpp
)

target_include_directories(Helios
    PUBLIC
        ${CMAKE_SOURCE_DIR}/Include
)

target_link_libraries(Helios PRIVATE raylib)

# ---- Game executable ----

add_executable(game
    Source/Game/main.cpp
)

target_link_libraries(game PRIVATE Helios)