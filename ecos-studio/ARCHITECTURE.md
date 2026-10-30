# ECOS Studio Architecture

## Product intent

ECOS Studio is a general-purpose game engine and creator-facing desktop editor. Its long-term target is a workflow and capability set in the class of Unreal Engine, suitable for ambitious AAA projects. This is a direction, not a claim of current parity.

## Product layers

- **Engine Core:** lifecycle, platform services, scene model, assets, jobs, input, time, and runtime APIs.
- **Renderer:** GPU resources, visibility, draw submission, materials, lighting, and render diagnostics.
- **World Runtime:** hierarchical regions/cells, streaming, LOD, floating origin, budgets, and simulation tiers.
- **Studio Editor:** viewport, hierarchy/outliner, inspectors, asset browser, import and material tools, undo/redo, debugging, profiling.
- **Game Runtime:** gameplay, physics, audio, scripting, and standalone execution independent of the editor.
- **Creator/Mod SDK:** versioned content formats, validation, compatibility and security boundaries.
- **Build and Validation:** reproducible builds, automated checks, sample game and platform validation.

## Implemented source-backed foundation

C++20 core and CLI; project/mod manifests; scene graph and serialization; asset registry and SHA-256 fingerprints; OBJ importer with generated normals and local mesh bounds; basic MTL parser; CPU reference renderer; native GLFW/OpenGL viewport; basic native Outliner/Inspector and scene persistence; input/camera/frame-loop primitives; AABB/sphere collision queries; software PCM mixer; world-cell lifecycle/manifest validation.

The native viewport has an early GPU path with shared mesh cache, frustum culling, instanced drawing where supported, a non-instanced fallback, viewport-driven cell updates, distant box proxies generated from local mesh bounds, and a simple directional shader term. Treat this as experimental until CI and visual/runtime validation are complete.

## Subsystem status

| Subsystem | Current state | Next meaningful capability |
|---|---|---|
| Core/project | Initial implementation | Stable runtime APIs, diagnostics and migration policy |
| Scene graph | Implemented foundation | Editor-facing scene tools and robust serialization workflow |
| Asset import | OBJ and basic MTL parser | Material linkage, textures, glTF and import diagnostics |
| GPU renderer | Experimental OpenGL compatibility path | Modern backend, resource lifetime and renderer tests |
| Visibility/draw | Cache, frustum culling, batching/instancing path | Profiling, correctness tests and scalable submission |
| Materials/lighting | Per-asset flat albedo and simple directional shader | Normal correctness, material resources, textures, PBR |
| World runtime | Cell lifecycle plus viewport-driven initial loading and LOD | Async I/O, scheduling, eviction, floating origin and telemetry |
| Editor | Native viewport, Outliner/Inspector basics, save and undo/redo | Asset browser, gizmos, material editor and reliable workflows |
| Physics | AABB/sphere queries, ECS rigid-body integration, contact resolution and collision events | Broadphase/solver scalability, advanced constraints and editor integration |
| Audio | Software PCM mixer | Device output, decoding and spatial audio |
| Scripting/gameplay | Not implemented as a complete runtime | Sandboxed API, reload, debugging and play-in-editor |
| Mod SDK | Data-only bootstrap and dependency resolution | Version compatibility, permissions and runtime mounts |
| Build/QA | CMake and multi-platform CI | Performance/visual regression and end-to-end sample game |

## Large-world architecture

The intended hierarchy is country → region → cell → local scene. The full ~1.7 million km² concept must never be loaded or simulated at uniform maximum fidelity. The renderer should prioritize nearby detailed meshes, represent distant content with cheaper LOD, and unload content outside budget. Integer cell coordinates help preserve precision; floating-origin integration and production async streaming remain future work.

## Mod security

Community mods remain data-only until a trust model defines permissions, API compatibility, signing/trust prompts, and isolation. Do not execute arbitrary mod code in the editor or game runtime by default.

## Engineering rules

1. Keep editor and shipped game runtime separate; the editor must not be required to run a game.
2. Do not rewrite stable cache/culling/streaming paths without a demonstrated reason.
3. A capability is not complete merely because its design is documented.
4. CI passing proves only the checks it executes; it does not prove visual quality or AAA performance.
