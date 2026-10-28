# ECOS Studio Roadmap

This roadmap describes product milestones, not guaranteed delivery dates.

## S0 — Repository foundation
- [x] C++20/CMake bootstrap CLI and project scaffolding.
- [x] Data-only mod scaffold creation and validation.
- [x] CI build and smoke workflow.

## S1 — Runtime and asset foundations (in progress)
- [x] Runtime lifecycle, logging, typed errors/results, portable platform helpers.
- [x] In-memory scene graph with hierarchical transforms and cycle-safe reparenting.
- [x] Versioned scene serialization/deserialization and scene validation CLI.
- [x] Asset registry with stable IDs, safe paths, and dependency validation.
- [x] Initial OBJ mesh importer with triangulation, index validation, and bounds.
- [x] World cell records, lifecycle transitions, resident-cell budget, and spatial radius query.
- [x] Headless .ecworld manifest validator and sample region/cell metadata.
- [ ] Asset content hashing, normal/UV import, and dependency-cycle diagnostics.

## S2 — First renderable desktop slice
- [x] Deterministic CPU reference rasterizer and OBJ-to-PPM CLI path.
- [ ] Window and input abstraction.
- [ ] GPU renderer backend and camera.
- [ ] Mesh/material GPU upload and visible interactive scene viewport.
- [ ] Basic audio output and spatial audio.
- [ ] Collision/rigid-body integration.
- [ ] One small offline game built and packaged by the toolchain.

## S3 — Creator tools
- [ ] Desktop editor shell and scene viewport.
- [ ] Asset browser, inspector, undo/redo.
- [ ] Import pipeline and reproducible game packaging.
- [ ] Scripting and debugging workflow.

## S4 — Large-world runtime
- [ ] Asynchronous cell I/O, worker queue, cancellation, prioritization, and eviction.
- [ ] LOD and floating-origin integration.
- [ ] Tiered simulation and streaming budget telemetry.

## S5 — Mod SDK
- [ ] Data-only mod package schema, dependency/compatibility resolver, load order and conflict reports.
- [ ] Permission model before executable extensions.

## S6 — ECOS flagship integration
- [ ] Import selected ECOS canon/world data.
- [ ] Build a representative small region with public tools.
- [ ] Expand regions and simulation systems incrementally.

## Quality bar
A capability remains labelled planned until source and automation support it. CI does not substitute for visual or performance validation on target PCs.
