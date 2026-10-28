# ECOS Studio

**ECOS Studio** is the independent game-development engine and modding toolkit being built for creators and for the ECOS PC flagship game, whose world concept spans approximately **1.7 million km²**.

This branch is an active C++20 engine foundation, not yet a complete UE5-equivalent product.

## Implemented source foundation
- Project and data-only mod scaffold creation/validation CLI.
- Runtime lifecycle, thread-safe logger, typed errors/results, and portable filesystem helpers.
- In-memory scene graph with parent-child hierarchy, local transforms, world matrices, cycle prevention, and subtree removal.
- Asset registry with stable IDs, safe project-relative paths, and dependency-reference validation.
- World cell records with integer coordinates, load-state transitions, resident/loading budget, failure diagnostics, retry, unload, and deterministic radius query.
- CMake build and GitHub Actions automation for the test suite.

## Still planned
No visible 3D renderer, desktop editor viewport, mesh importer, physics runtime, audio engine, scripting VM, or asynchronous world streaming is implemented yet. The next work is versioned serialization and a headless world validator, followed by a real graphics/window slice and creator-facing editor.

## Build
Requires CMake 3.20+ and a C++20 compiler.

    cmake -S . -B build
    cmake --build build --config Release
    ctest --test-dir build --output-on-failure

## CLI
    ecos-studio --help
    ecos-studio new ../MyGame "My Game"
    ecos-studio validate ../MyGame
    ecos-studio mod-init ../MyGameMod com.example.my-mod
    ecos-studio mod-validate ../MyGameMod

## Engineering principles
1. Build an independent architecture; do not copy Unreal Engine source.
2. Stream large worlds instead of keeping the whole country active in RAM.
3. Use versioned data formats and explicit migrations before freezing public APIs.
4. Keep mods data-only until a proper extension trust model exists.
5. Mark every capability implemented, experimental, or planned honestly.
