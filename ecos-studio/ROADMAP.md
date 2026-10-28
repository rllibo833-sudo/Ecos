# ECOS Studio Roadmap

This roadmap describes product milestones, not guaranteed delivery dates.

## S0 — Repository foundation
- [x] Dedicated branch and product scope.
- [x] C++20/CMake bootstrap CLI.
- [x] Project scaffold creation and manifest validation.
- [x] Mod scaffold creation and validation.
- [x] CI build and smoke workflow.

## S1 — Runtime foundations (in progress)
- [x] Runtime lifecycle, logging, typed errors/results, portable platform helpers.
- [x] In-memory scene graph with hierarchical transforms and cycle-safe reparenting.
- [x] Asset registry with stable IDs, safe paths, and dependency validation.
- [x] World cell records, lifecycle transitions, resident-cell budget, and spatial radius query.
- [ ] Versioned serialized scene/asset/world schemas and migration utilities.
- [ ] Headless world-data validator and asset content hashing.

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
- [ ] Asynchronous cell I/O, worker queue, cancellation, and prioritization.
- [ ] Eviction policy, LOD, and floating-origin integration.
- [ ] Tiered simulation and streaming budget telemetry.

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
