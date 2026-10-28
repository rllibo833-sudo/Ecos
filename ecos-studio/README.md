# ECOS Studio

**ECOS Studio** is the planned commercial game-development engine and modding toolkit for independent developers, studios, and ECOS community modders. Its flagship project is the ECOS PC game, designed around a country-scale world of approximately **1.7 million km²**.

This branch establishes the first source-controlled foundation. It does **not** claim to be feature-complete or equivalent to Unreal Engine yet.

## Product goals

- A desktop editor for building games and authoring worlds.
- A reusable runtime/core that can ship games independently of the editor.
- World tools designed for large worlds: partitioning, streaming, hierarchical detail, and region-scoped simulation.
- Physics, audio, input, rendering, animation, scripting, asset import, profiling, and packaging as separate subsystems.
- A versioned mod SDK with compatibility rules, content packages, and creator documentation.
- ECOS as the flagship game built with the same tools available to other developers.

## Current implementation

The current bootstrap is a dependency-free C++20 command-line tool. It can create and validate project scaffolds and mod-package scaffolds. It does **not** yet render 3D scenes, simulate physics, play audio, or build a playable game. Those are explicit future engine milestones, not implied features.

## Build

Requires CMake 3.20+ and a C++20 compiler.

    cmake -S . -B build
    cmake --build build --config Release

Run the executable from the build output directory:

    ecos-studio new ../MyGame "My Game"
    ecos-studio validate ../MyGame
    ecos-studio mod-init ../MyGameMod com.example.my-mod
    ecos-studio mod-validate ../MyGameMod

On multi-configuration generators, the executable may be under build/Release/.

## Project and mod formats

Projects use a deliberately small, dependency-free project.ecos key/value manifest. Mod packages use mod.ecos. Both formats are versioned. See docs/PROJECT-FORMAT.md and modding/MOD-SDK.md.

## Product principles

1. **One engine, two consumers:** ECOS Studio for third-party creators and ECOS the game.
2. **Stream, don't load everything:** large-world runtime must load regions and simulation tiers on demand.
3. **Creator-first:** reproducible projects, clear errors, stable formats, and documented migration.
4. **Commercially sustainable:** maintain a third-party license inventory before adopting external code or assets.
5. **Evidence over claims:** every capability is labelled implemented, experimental, or planned.
