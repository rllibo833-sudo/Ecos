# ECOS Studio — Engineering Roadmap

Dokumen ini adalah satu-satunya roadmap engineering kanonis untuk branch `ecos-studio`. Status menggambarkan source saat ini; CI dan validasi visual dicatat terpisah. Target produk adalah studio editor mandiri dengan cakupan kemampuan kelas Unreal Engine untuk game berambisi AAA, bukan klaim bahwa parity sudah tercapai.

## R0 — Core dan format data

- [x] C++20/CMake bootstrap dan project scaffold.
- [x] Manifest project dan data-only mod package.
- [x] Scene graph, transform hierarkis, serialisasi scene, dan validator.
- [x] Registry asset, dependency validation, fingerprint SHA-256.
- [x] OBJ import, triangulasi, UV/normals, validasi geometri, dan local bounds.
- [x] Parser teks MTL dasar.
- [x] CI lintas platform dan automated regression tests.

## R1 — Native editor dan viewport (partial/experimental)

- [x] Native GLFW/OpenGL window, grid, axes, depth-tested viewport, orbit/zoom.
- [x] Preview mesh OBJ dan render scene project melalui registry asset.
- [x] World Outliner/Inspector dasar, entity create/delete, transform editing, undo/redo, save.
- [x] GPU mesh cache, frustum culling, batching/instancing path dan fallback.
- [x] Viewport position memicu update awal world-cell runtime.
- [x] LOD jauh menggunakan box proxy dari local mesh bounds, bukan ukuran tetap.
- [x] Shader directional shading sederhana dengan formula `uAlbedo * (0.7 + 0.3 * NdotL)`.
- [ ] Verifikasi visual di perangkat target dan regression checks untuk ukuran proxy, warna, normals, serta jalur instancing/fallback.
- [ ] Renderer modern dengan resource/material pipeline yang tahan produksi.

## R2 — Material dan rendering

- [ ] Material resources dan assignment UI di editor.
- [ ] Texture import, sampling, color-space handling, dan asset references.
- [ ] Normal/tangent pipeline yang konsisten untuk model transform dan lighting.
- [ ] PBR workflow, shadow maps, environment lighting, tone mapping, anti-aliasing.
- [ ] Renderer profiling dan visual regression scenes.

## R3 — World runtime skala besar

- [~] World manifest dan cell lifecycle foundation.
- [~] Viewport-driven initial cell update dan LOD proxy path.
- [ ] Async disk I/O, worker scheduling, prioritization, cancellation, retries, eviction.
- [ ] Floating origin, region hierarchy, streaming budgets, telemetry.
- [ ] Multi-tier simulation untuk objek jauh dan deterministik world persistence.
- [ ] Stress test untuk banyak cell/mesh dan world berskala besar.

## R4 — Workflow studio

- [x] Scene hierarchy dan transform inspector dasar.
- [ ] Asset browser persisten, import queue, validation reports.
- [ ] Transform gizmos, selection/picking, multi-select dan editor camera workflow.
- [ ] Material editor dan preview.
- [ ] Undo/redo yang mencakup seluruh operasi konten dan recovery/autosave.
- [ ] World/terrain authoring tools dan prefab/instancing workflow.

## R5 — Runtime gameplay

- [~] Standalone native game executable with a playable sample (ECOS Rally); project-authored game packaging and editor play-in-editor remain open.

- [x] Frame loop, input snapshots, camera math, query collision, ECS rigid-body simulation/contact response, collision events, software PCM mixer.
- [x] Physics simulation dasar: ECS rigid-body integration, contact resolution, friction, dan collision events.
- [ ] Audio device backend, decoding, spatial audio.
- [ ] Sandboxed scripting, gameplay APIs, hot reload, debugging.
- [ ] Play-in-editor, runtime inspection, profiling dan crash diagnostics.

## R6 — Creator ecosystem dan production pipeline

- [x] Data-only mod scaffold dan dependency resolver lokal.
- [ ] API/format version compatibility, mount rules, conflict reports dan permission model.
- [ ] Build cook pipeline, reproducible game builds dan target platform validation.
- [~] Standalone ECOS Rally vertical slice: separate executable, fixed-step/input/ECS/AABB gameplay, win/restart loop, packaged target, and Linux virtual-display smoke test. Asset-driven project cooking and editor-to-runtime deployment remain open.
- [ ] Creator SDK docs dan migration policy yang mengikuti implementasi.

## Kriteria kemajuan

- Fitur baru menambah kemampuan nyata, bukan hanya dokumen atau tes tambahan.
- Setiap perubahan menjaga fitur lama dan mencatat bukti verifikasi.
- Status CI terbaru dilaporkan dari workflow aktual, bukan diasumsikan dari roadmap.
- Klaim kualitas AAA memerlukan profiling, pengujian visual, stabilitas, konten dan workflow produksi—bukan sekadar build hijau.
