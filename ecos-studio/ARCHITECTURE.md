# ECOS Studio Architecture

## Product split

- **ECOS Core:** runtime services shared by editor and shipped games.
- **ECOS Studio Editor:** desktop application for scenes, world regions, assets, scripts, debugging, profiling, and packaging.
- **ECOS Mod SDK:** stable, versioned interfaces for creators.
- **ECOS Game:** flagship game and demanding consumer of the engine.

The editor must not be required to run a packaged game. Runtime and editor share project and asset formats but have separate launch targets.

## Implemented foundation

The C++20 core includes project/mod scaffold tools, runtime lifecycle/logging, typed errors, filesystem helpers, scene graph and serialization, asset dependency validation and cycle detection, SHA-256 content fingerprints, OBJ import with UVs/normals, a basic MTL material parser, a CPU reference rasterizer, world-cell lifecycle and manifest validation, a platform-neutral input event queue/state, and validated camera/projection math, collision overlap/ray queries for AABBs and spheres, deterministic PCM audio mixer, backend-neutral input state, and a bounded fixed-step frame clock.

CI builds the code and runs automated tests, renders a sample mesh to PPM, and validates sample project/mod/scene/world artifacts. This is a growing engine foundation, not a production desktop editor or GPU renderer.

## Subsystem boundaries

| Subsystem | Responsibility | Status |
|---|---|---|
| Runtime/platform | Lifecycle, logging, errors, filesystem basics | Initial implementation |
| Scene graph | Nodes, local/world transforms, hierarchy, serialization | Initial implementation |
| Asset registry | Stable IDs, safe paths, dependency references and cycles | Initial implementation |
| Content identity | SHA-256 fingerprints of exact bytes | Implemented utility; cache not yet integrated |
| World runtime | Cell lifecycle, budgets, spatial query | Initial implementation; async I/O planned |
| Project/mod manifests | Scaffold creation and validation | Bootstrap implemented |
| Mesh import | OBJ positions, UVs, normals, triangulation, bounds | Initial implementation |
| Reference renderer | CPU rasterization, depth buffer, simple diffuse lighting, PPM | Initial implementation |
| Input | Platform-neutral queued events and per-frame state | Initial implementation; no OS adapter |
| Camera math | View, perspective/orthographic projection, NDC conversion | Initial implementation |
| GPU renderer/window | Graphics backend, frame pipeline, native window | Planned |
| Physics queries | AABB/sphere overlap and ray intersection | Initial implementation |
| Physics simulation | Rigid bodies, constraints, collision response and stepping | Planned |
| Audio | Software PCM mixing, gain, pan and voice state | Initial implementation; device output/spatialization planned |
| Asset pipeline | OBJ import, basic MTL material parsing, content hashes | Partial foundation; OBJ/MTL linkage, textures and cache planned |
| Editor | Viewport, inspectors, undo/redo, world tools | Planned |
| Scripting | Gameplay API, sandbox and reload workflow | Planned |
| Mod runtime | Version checks, content mounts and permissions | Planned |
| Packaging | Reproducible builds and platform bundles | Planned |

## Large-world constraints

The ECOS country-scale world (~1.7 million km²) must not be fully resident in RAM or simulated at full fidelity. The intended hierarchy is country → region → cell → local scene, with asynchronous streaming, LOD, floating-origin coordinates, region-level distant simulation, and explicit memory/entity/streaming budgets. Current world code establishes cell IDs, integer coordinates, state transitions, a resident budget, and radius queries; asynchronous disk I/O and production LOD are not implemented.

## Mod security

Mods start as data-only packages with schema validation, dependency declarations, version constraints, and explicit asset mount permissions. Executable extensions require a separate trust model.

## Implementation rule

Unreal Engine 5 is a capability target, not a codebase to copy or a parity claim. Each subsystem is built and documented independently.
