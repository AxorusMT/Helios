# Helios

Helios is a small C++23 game engine experiment built with raylib.

The project currently has:

- A basic application loop.
- A layer stack with attach, detach, update, and draw hooks.
- Test layers for simple rendering and input checks.

## Build

```powershell
cmake -S . -B build
cmake --build build
```

The game executable is built from `Source/Game/main.cpp`.
