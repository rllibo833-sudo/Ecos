# ECOS Studio Architecture

## Product split
- **ECOS Core:** runtime services shared by editor and shipped games.
- **ECOS Studio Editor:** desktop application for scenes, world regions, assets, scripts, debugging, profiling, and packaging.
- **ECOS Mod SDK:** stable, versioned interfaces for creators.
- **ECOS Game:** flagship game and demanding consumer of the engine.

The editor must not be required to run a packaged game. Runtime and editor share project and asset formats but have separate launch targets.

## Implemented foundation
The C++20 core includes project/mod scaffold CLI tools, runtime lifecycle and logging, typed errors/results, portable filesystem/path helpers, an in-memory scene graph with hierarchical transforms and cycle rejection, an asset registry with dependency validation, a world-cell lifecycle model with loading budgets and spatial radius queries, a validated OBJ importer, and a deterministic CPU reference triangle renderer that emits PPM images.

CI builds the C++ code and runs automated tests. The CPU renderer proves a basic mesh-to-pixels path but is not a GPU renderer or a desktop editor. Production viewport, physics, audio, and scripting runtime are not implemented yet.

## Subsystem boundaries
| Subsystem | Responsibility | Status |
|---|---|---|
| Runtime/platform | Lifecycle, logging, errors, filesystem basics | Initial implementation |
| Scene graph | Nodes, local/world transforms, hierarchy | Initial implementation |
| Asset registry | Stable IDs, paths, dependencies | Initial implementation |
| World runtime | Cell lifecycle, budget, spatial query | Initial implementation |
| Project/mod manifests | Bootstrap creation and validation | Implemented bootstrap |
| Mesh import | OBJ positions, triangulation, bounds | Initial implementation |
| Reference renderer | CPU triangle rasterization, depth buffer, diffuse light, PPM output | Initial implementation |
| GPU renderer | Graphics API abstraction, frame pipeline, materials, visibility | Planned |
| Physics/audio | Collision, rigid bodies, audio output and spatial audio | Planned |
| Asset pipeline | Importers, metadata, content hashes, cache | Planned |
| Editor | Viewport, inspectors, undo/redo, world tools | Planned |
| Scripting | Gameplay API, sandbox and reload workflow | Planned |
| Mod runtime | Version checks, content mounts and permissions | Planned |
| Packaging | Reproducible builds and platform bundles | Planned |

## Large-world constraints
The ECOS country-scale world (~1.7 million km²) must not be fully resident in RAM or simulated at full fidelity. The intended hierarchy is country → region → cell → local scene, with asynchronous streaming, LOD, floating-origin coordinates, region-level distant simulation, and explicit memory/entity/streaming budgets. Current world code establishes cell IDs, integer coordinates, state transitions, a resident budget, and radius queries; asynchronous disk I/O is not implemented yet.

## Mod security
Mods start as data-only packages with schema validation, dependency declarations, version constraints, and explicit asset mount permissions. Executable extensions require a separate trust model.

## Implementation rule
Unreal Engine 5 is a capability target, not a codebase to copy or a parity claim. Each subsystem is built and documented independently.
