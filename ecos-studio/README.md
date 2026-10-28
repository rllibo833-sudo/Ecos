# ECOS Studio

**ECOS Studio** is an independent C++20 engine foundation and modding toolkit for creators and the ECOS PC flagship game, whose world concept spans approximately **1.7 million km²**.

It is not yet a complete Unreal Engine 5-equivalent product. Implemented capabilities below are source-backed and covered by automated tests where noted.

## Implemented

- Project and data-only mod scaffold creation/validation CLI, with validated dependency declarations for third-party creators.
- Deterministic mod dependency resolver with installed-package discovery, missing-dependency diagnostics, cycle detection, and stable dependency-first ordering.
- OBJ asset import into an existing project, with geometry validation, preserved source bytes, SHA-256 content fingerprint, mesh bounds/counts, and a versioned `.ecasset` sidecar.
- Imported-asset integrity validation: metadata schema, path confinement, source hashes, OBJ re-parsing, and geometry-statistic verification.
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
- Deterministic software PCM mixer for mono/stereo clips, gain, pan, looping, stopping voices, and output clipping; no device output yet.
- Backend-neutral input snapshots for keyboard, mouse, cursor delta, scroll, and quit events.
- Bounded fixed-step frame clock with delta clamping, catch-up limits, and render interpolation alpha.
- Backend-neutral frame loop integrating fixed updates, stable input snapshots, and one render callback per frame.
- CMake build, subsystem tests, and GitHub Actions automation.

## Modding for every creator

ECOS Studio is intended as a general-purpose engine and creator toolkit, not an ECOS-game-only editor. Mod packages use `mod.ecos`; declare dependencies as `dependencies=none` or a comma-separated list such as `dependencies=com.vendor.base,com.example.content`.

The `mod-resolve` command scans immediate child directories under a mods root, validates each discovered package, reports missing dependencies and dependency cycles, and prints a deterministic dependency-first order:

~~~
ecos-studio mod-resolve ../MyGame/mods
~~~

Resolution is local-only: packages must already exist on disk. There is no package download, version-range solver, runtime loading, or executable scripting yet. Mods remain data-only for safety.

## Importing and validating assets

Import a valid Wavefront OBJ into a project. The importer copies the original file into `assets/imported/` and creates a `.ecasset` metadata file containing the asset ID, type, SHA-256 fingerprint, vertex/triangle counts, and bounds.

~~~
ecos-studio asset-import ../MyGame ./models/character.obj character.mesh
ecos-studio asset-validate ../MyGame
~~~

Validation re-hashes source bytes, reparses the OBJ, checks stored geometry statistics, and rejects unsafe metadata paths and symlink entries. Imports refuse to overwrite existing files. MTL parsing is available as a separate foundation; automatic OBJ-to-MTL linking and texture import are not yet wired into this command.

## Not implemented yet

No production GPU backend, native desktop window/editor shell, interactive viewport, physics runtime, audio device backend, scripting VM, asynchronous disk streaming, or complete material/import pipeline exists yet. Camera math and input state are foundations for those integrations, not proof that a viewport is available.

## Build

Requires CMake 3.20+ and a C++20 compiler.

~~~
cmake -S ecos-studio -B build
cmake --build build --config Release
ctest --test-dir build --output-on-failure
~~~

## CLI

~~~
ecos-studio --help
ecos-studio new ../MyGame "My Game"
ecos-studio validate ../MyGame
ecos-studio mod-init ../MyGameMod com.example.my-mod
ecos-studio mod-validate ../MyGameMod
ecos-studio mod-resolve ../MyGame/mods
ecos-studio asset-import ../MyGame ./models/character.obj character.mesh
ecos-studio asset-validate ../MyGame
ecos-studio scene-validate examples/hello-world/scenes/main.ecscene
ecos-studio world-validate examples/large-world/world.ecworld
~~~

## Engineering principles

1. Build an independent architecture; do not copy Unreal Engine source.
2. Stream large worlds instead of keeping the whole country active in RAM.
3. Use versioned data formats and explicit migrations before freezing public APIs.
4. Keep mods data-only until a proper extension trust model exists.
5. Mark every capability implemented, experimental, or planned honestly.
