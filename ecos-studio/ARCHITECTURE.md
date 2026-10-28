# ECOS Studio Architecture

## Product split
- **ECOS Core:** runtime services shared by the editor and shipped games.
- **ECOS Studio Editor:** desktop application for scenes, world regions, assets, scripts, debugging, profiling, and packaging.
- **ECOS Mod SDK:** stable, versioned interfaces for content creators and mod authors.
- **ECOS Game:** the flagship game and the first demanding consumer of the engine.

The editor must not be required to run a packaged game. The runtime and editor share project and asset formats but have separate launch targets.

## Implemented bootstrap and core
The current C++20 implementation includes:
- A command-line project and data-only mod scaffold generator/validator.
- A minimal runtime lifecycle state machine (cold → ready → stopped).
- A mutex-protected log writer with debug, info, warning, and error levels.
- Stable error codes and Result<T> / Result<void> types for explicit failure handling.
- Portable utilities for monotonic time, OS identification, safe project-relative paths, directory creation, and text-file reads.
- Automated tests for lifecycle, logging, path traversal rejection, filesystem reads, and platform utilities.

This is foundational infrastructure, not a complete runtime. It does not yet own a window, render graphics, simulate physics, or output audio.

## Planned subsystem boundaries
| Subsystem | Responsibility | Status |
|---|---|---|
| Runtime lifecycle and logger | Minimal lifecycle and synchronized log output | Implemented |
| Error/result and diagnostics | Typed error codes and explicit operation results | Initial implementation |
| Platform layer | Clock, OS identification, safe paths, basic filesystem | Initial implementation |
| Project and mod manifests | Create/validate project and package metadata | Bootstrap implemented |
| Renderer | RHI abstraction, frame graph, materials, visibility | Planned |
| World runtime | Region graph, streaming, LOD, origin rebasing | Planned |
| Physics | Broadphase, collision, rigid bodies, character controller | Planned |
| Audio | Device/output, spatial audio, buses, streaming | Planned |
| Asset pipeline | Import, metadata, cache, dependency graph | Planned |
| Editor | Scene/world tools, inspectors, undo/redo, viewport | Planned |
| Scripting | Sandboxed gameplay API and reload workflow | Planned |
| Mod runtime | Version checks, content mount points, permissions | Planned |
| Packaging | Reproducible builds, platform bundles, diagnostics | Planned |

## Large-world design constraints
A 1.7-million-km² fictional country is a content scale, not a requirement to keep every object resident in RAM or active in simulation. The runtime should use:
- A hierarchical world index: country → region → cell → local scene.
- Asynchronous cell streaming and deterministic load/unload boundaries.
- Multiple levels of detail for geometry, vegetation, crowds, and simulation.
- Region-level simulation for distant regions and higher fidelity near the player.
- Stable geographic coordinates plus local floating-origin coordinates for rendering and physics.
- Explicit budgets for memory, active entities, draw calls, streaming bandwidth, and background jobs.

The initial prototype should prove data formats and streaming transitions before advanced graphics are attempted.

## Mod security
The bootstrap does not execute mod code. Future mods should begin as data-only packages with schema validation, dependency declarations, version constraints, and explicit asset mount permissions. Native plugins, if ever supported, require a separate trust model and user consent.

## Implementation rule
Do not claim parity with Unreal Engine from the presence of a directory, interface mock-up, or line count. Parity is a long-term product ambition; subsystems must be implemented and documented individually.
