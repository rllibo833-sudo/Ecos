# ECOS Studio Roadmap

This roadmap describes product milestones, not guaranteed delivery dates.

## S0 — Repository foundation
- [x] Dedicated branch and product scope.
- [x] C++20/CMake bootstrap CLI.
- [x] Project scaffold creation and manifest validation.
- [x] Mod scaffold creation and manifest validation.
- [x] CI build and smoke workflow.

## S1 — Runtime foundations (in progress)
- [x] Minimal runtime lifecycle state machine.
- [x] Thread-safe log writer with severity levels.
- [x] Stable error codes and value/error result types.
- [x] Portable platform utilities and project-relative path safety.
- [x] In-memory scene graph with hierarchical transforms and cycle-safe reparenting.
- [x] Asset registry with stable IDs, normalized paths, and dependency validation.
- [ ] Versioned serialized scene/asset schemas and migration utilities.
- [ ] Headless world-data validator and region/cell manifest.

## S2 — First playable desktop slice
- [ ] Window and input abstraction.
- [ ] Renderer backend and camera.
- [ ] Mesh/material loading and visible scene viewport.
- [ ] Basic audio output and spatial audio.
- [ ] Collision/rigid-body integration.
- [ ] One small offline game built and packaged by the toolchain.

## S3 — Creator tools
- [ ] Desktop editor shell and scene viewport.
- [ ] Asset browser, inspector, scene hierarchy, undo/redo.
- [ ] Import pipeline and reproducible game packaging.
- [ ] Scripting and debugging workflow.

## S4 — Large-world runtime
- [ ] Region/cell world format and spatial index.
- [ ] Asynchronous cell streaming and eviction.
- [ ] LOD and floating-origin strategy.
- [ ] Tiered simulation and explicit streaming budgets.

## S5 — Mod SDK
- [ ] Data-only mod package schema.
- [ ] Dependency and compatibility resolver.
- [ ] Mod load order and conflict reports.
- [ ] Permission model before executable extensions.

## S6 — ECOS flagship integration
- [ ] Import selected ECOS canon/world data.
- [ ] Build a representative small region with public tools.
- [ ] Expand regions and simulation systems incrementally.

## Quality bar
A capability remains labelled planned until source and automation support it. CI does not substitute for visual or performance validation on target PCs.
