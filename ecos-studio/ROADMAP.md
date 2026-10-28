# ECOS Studio Roadmap

This roadmap describes product milestones, not guaranteed delivery dates. Checkboxes represent code that exists, not aspirations.

## S0 — Repository foundation
- [x] C++20/CMake bootstrap CLI and project scaffolding.
- [x] Data-only mod scaffold creation and validation.
- [x] CI build and smoke workflow.

## S1 — Runtime, scene, and asset foundations (in progress)
- [x] Runtime lifecycle, logging, typed errors/results, portable platform helpers.
- [x] In-memory scene graph with hierarchical transforms and cycle-safe reparenting.
- [x] Versioned scene serialization/deserialization and scene validation CLI.
- [x] Asset registry with stable IDs, safe paths, dependency validation, and cycle diagnostics.
- [x] SHA-256 content fingerprints for exact asset bytes.
- [x] OBJ importer with triangulation, independent UV/normal indices, seam expansion, validation, bounds, and generated normals.
- [x] MTL material text parser for diffuse color, opacity/transparency, shininess, and illumination model.
- [x] World cell records, lifecycle transitions, resident-cell budget, and spatial radius query.
- [x] Headless .ecworld manifest validator and sample region/cell metadata.

## S2 — First renderable desktop slice (in progress)
- [x] Deterministic CPU reference rasterizer and OBJ-to-PPM CLI path.
- [x] Platform-neutral input event/state abstraction.
- [x] Camera view/projection and world-to-NDC math.
- [x] AABB/sphere overlap and ray intersection query primitives.
- [x] Deterministic software PCM mixer with mono/stereo, gain, pan, loops, and voice state.
- [x] Backend-neutral keyboard/mouse input snapshots and per-frame transient reset.
- [x] Bounded fixed-step frame clock with interpolation alpha.
- [x] Callback-driven frame loop with fixed updates, shared input snapshot, and render callback.
- [ ] Native window and OS input adapter.
- [ ] GPU renderer backend and frame lifecycle.
- [ ] Mesh/material GPU upload and visible interactive scene viewport.
- [ ] OS audio-device output, decoding/resampling, and spatial audio.
- [ ] Collision/rigid-body integration.
- [ ] One small offline game built and packaged by the toolchain.

## S3 — Creator tools
- [ ] Desktop editor shell and scene viewport.
- [ ] Asset browser, inspector, undo/redo.
- [ ] Material editor, import pipeline, and reproducible game packaging.
- [ ] Scripting and debugging workflow.

## S4 — Large-world runtime
- [ ] Asynchronous cell I/O, worker queue, cancellation, prioritization, and eviction.
- [ ] LOD and floating-origin integration.
- [ ] Tiered simulation and streaming budget telemetry.

## S5 — Mod SDK
- [ ] Data-only mod package schema, dependency/compatibility resolver, load order, and conflict reports.
- [ ] Permission model before executable extensions.

## S6 — ECOS flagship integration
- [ ] Import selected ECOS canon/world data.
- [ ] Build a representative small region with public tools.
- [ ] Expand regions and simulation systems incrementally.

## Quality bar

A capability remains labelled planned until source and automation support it. CI does not substitute for visual or performance validation on target PCs.
