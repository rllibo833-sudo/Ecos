# ECOS Studio

**ECOS Studio** is an independent C++20 engine foundation and modding toolkit for creators and the ECOS PC flagship game, whose world concept spans approximately **1.7 million km²**.

It is not yet a complete Unreal Engine 5-equivalent product. Implemented capabilities below are source-backed and covered by automated tests where noted.

## Implemented

- Project and data-only mod scaffold creation/validation CLI.
- Runtime lifecycle, thread-safe logging, typed errors/results, and safe relative-path/filesystem helpers.
- Scene graph with hierarchical transforms, cycle prevention, subtree removal, versioned serialization, and validation CLI.
- Asset registry with stable IDs, dependency reference/cycle validation, and SHA-256 content fingerprints.
- Wavefront OBJ importer supporting positions, UVs, explicit/generated normals, independent positive/negative indices, polygon fan triangulation, UV seam expansion, finite-value validation, and mesh bounds.
- Wavefront MTL text importer for named materials, diffuse color, opacity/transparency, shininess, and illumination model.
- Deterministic CPU reference triangle rasterizer with depth buffering, simple directional lighting, and PPM output. This is a test/reference renderer, not a GPU viewport.
- World-cell lifecycle model, resident/loading budgets, failure/retry/unload, and radius queries.
- .ecworld manifest validator for schema version, unique cell IDs/coordinates, and safe content paths.
- Platform-neutral input event queue/state for keys, mouse buttons, pointer movement, wheel deltas, and focus-loss releases.
- Camera view, perspective/orthographic projection, and projection of points to NDC with validation.
- Collision query foundation with AABB/sphere overlap, ray-versus-AABB, and ray-versus-sphere tests.
- CMake build, subsystem tests, and GitHub Actions automation.

## Not implemented yet

No production GPU backend, native desktop window/editor shell, interactive viewport, physics runtime, audio engine, scripting VM, asynchronous disk streaming, or complete material/import pipeline exists yet. Camera math and input state are foundations for those integrations, not proof that a viewport is available.

## Build

Requires CMake 3.20+ and a C++20 compiler.

~~~sh
cmake -S ecos-studio -B build
cmake --build build --config Release
ctest --test-dir build --output-on-failure
~~~

## CLI

~~~sh
ecos-studio --help
ecos-studio new ../MyGame "My Game"
ecos-studio validate ../MyGame
ecos-studio mod-init ../MyGameMod com.example.my-mod
ecos-studio mod-validate ../MyGameMod
ecos-studio scene-validate examples/hello-world/scenes/main.ecscene
ecos-studio world-validate examples/large-world/world.ecworld
~~~

## Engineering principles

1. Build an independent architecture; do not copy Unreal Engine source.
2. Stream large worlds instead of keeping the whole country active in RAM.
3. Use versioned data formats and explicit migrations before freezing public APIs.
4. Keep mods data-only until a proper extension trust model exists.
5. Mark every capability implemented, experimental, or planned honestly.
