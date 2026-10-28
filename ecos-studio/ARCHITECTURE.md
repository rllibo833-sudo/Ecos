# ECOS Studio Architecture

## Product split
- **ECOS Core:** runtime services shared by editor and shipped games.
- **ECOS Studio Editor:** desktop application for scenes, world regions, assets, scripts, debugging, profiling, and packaging.
- **ECOS Mod SDK:** stable, versioned interfaces for creators.
- **ECOS Game:** flagship game and demanding consumer of the engine.

The editor must not be required to run a packaged game. Runtime and editor share project and asset formats but have separate launch targets.

## Implemented foundation
The C++20 core currently includes project/mod scaffold CLI tools, lifecycle and logging, typed error/result values, portable filesystem/path helpers, an in-memory scene graph with hierarchical transforms, cycle rejection and subtree deletion, and an asset registry with stable IDs and dependency validation.

Automated CI tests cover these systems. This remains engine foundation work: no visible 3D renderer, full editor, physics, audio, or scripting runtime is implemented yet.

## Subsystem boundaries
| Subsystem | Responsibility | Status |
|---|---|---|
| Runtime lifecycle and logger | Lifecycle and synchronized log output | Implemented |
| Error/result and platform | Error codes, safe paths, clock, filesystem basics | Initial implementation |
| Scene graph | Nodes, local/world transforms, parent-child relationships | Initial implementation |
| Asset registry | Stable IDs, paths, declared dependencies | Initial implementation |
| Project/mod manifests | Bootstrap creation and validation | Implemented bootstrap |
| Renderer | Graphics API abstraction, frame pipeline, materials, visibility | Planned |
| World runtime | Region graph, streaming, LOD, origin rebasing | Planned |
| Physics/audio | Collision, rigid bodies, audio output and spatial audio | Planned |
| Asset pipeline | Importers, metadata, content hashes, cache, dependency graph | Planned |
| Editor | Viewport, inspectors, undo/redo, world tools | Planned |
| Scripting | Gameplay API, sandbox and reload workflow | Planned |
| Mod runtime | Version checks, content mounts and permissions | Planned |
| Packaging | Reproducible builds and platform bundles | Planned |

## Large-world constraints
The ECOS country-scale world (~1.7 million km²) must not be fully resident in RAM or simulated at full fidelity. The intended hierarchy is country → region → cell → local scene, with asynchronous streaming, LOD, floating-origin coordinates, region-level distant simulation, and explicit memory/entity/streaming budgets.

## Mod security
Mods start as data-only packages with schema validation, dependency declarations, version constraints, and explicit asset mount permissions. Executable extensions require a separate trust model.

## Implementation rule
Unreal Engine 5 is a capability target, not a codebase to copy or a parity claim. Each subsystem is built and documented independently.
