# ECOS Studio Roadmap

This roadmap describes product milestones, not guaranteed delivery dates.

## S0 — Repository foundation (current)

- [x] Dedicated branch and product scope.
- [x] C++20/CMake bootstrap CLI.
- [x] Project scaffold creation and manifest validation.
- [x] Mod scaffold creation and manifest validation.
- [x] CI build and smoke workflow.
- [x] Explicit status labels to distinguish shipped bootstrap from planned engine features.

## S1 — Runtime foundations

- [ ] Logging, diagnostics, platform abstraction, and stable error codes.
- [ ] Versioned project/asset schemas and migration utilities.
- [ ] Asset registry and dependency graph.
- [ ] Headless world-data validator.

## S2 — First playable desktop slice

- [ ] Window and input abstraction.
- [ ] Basic renderer and camera.
- [ ] Scene graph and transform hierarchy.
- [ ] Basic audio output and spatial audio.
- [ ] Basic collision/rigid-body integration.
- [ ] One small offline game built and packaged by the toolchain.

## S3 — Creator tools

- [ ] Desktop editor shell and scene viewport.
- [ ] Asset browser, inspector, scene hierarchy, undo/redo.
- [ ] Import pipeline and reproducible game packaging.
- [ ] Scripting and debugging workflow.

## S4 — Large-world runtime

- [ ] Region/cell world format and spatial index.
- [ ] Asynchronous streaming and eviction.
- [ ] LOD and floating-origin strategy.
- [ ] Tiered simulation for distant regions.
- [ ] Streaming budget instrumentation.

## S5 — Mod SDK

- [ ] Data-only mod package schema.
- [ ] Dependency and compatibility resolver.
- [ ] Mod load order and conflict reports.
- [ ] Creator docs and sample mods.
- [ ] Permission model before executable extensions.

## S6 — ECOS flagship integration

- [ ] Convert selected ECOS canon/world data into engine-importable data.
- [ ] Build a representative small region using the public tools.
- [ ] Expand regions and simulation systems incrementally.
- [ ] Keep engine features reusable by external developers.

## Quality bar

Each milestone must have source, documentation, and reproducible automation where possible. A feature remains labelled planned until implemented; automated CI is not a substitute for full visual/performance validation on target PCs.
