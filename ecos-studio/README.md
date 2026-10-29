# ECOS Studio

**ECOS Studio** adalah engine dan studio editor game general-purpose berbasis C++20. Tujuan jangka panjangnya adalah membangun workflow editor dengan kedalaman dan cakupan kelas Unreal Engine untuk game berambisi AAA, sambil tetap mandiri dalam arsitektur dan implementasi. Ini merupakan target, bukan klaim parity atau kesiapan produksi saat ini.

## Status saat ini

Branch ini bukan lagi CLI-only: terdapat native desktop editor awal dengan viewport OpenGL, pemuatan scene project, mesh import, scene editing dasar, dan jalur awal untuk menampilkan isi world-cell.

### Editor dan viewport native — Partial/Experimental

- Native GLFW/OpenGL desktop window dengan depth-tested 3D viewport, grid, sumbu XYZ, orbit/zoom, dan reset kamera.
- Mode preview mesh OBJ dan rendering scene `.ecscene` melalui scene graph serta registry asset yang tervalidasi.
- Native World Outliner dan Inspector dasar: pilih, buat, hapus entity; edit nama dan transform lokal; undo/redo; simpan scene.
- Jalur viewport menggunakan GPU mesh cache, frustum culling, batching/instancing bila tersedia, dan fallback non-instanced.
- World-cell runtime dapat diperbarui berdasarkan posisi viewport; konten cell yang termuat diproyeksikan ke koordinat dunia.
- LOD jarak jauh menggunakan box proxy berdasarkan bounds mesh dan shader directional shading sederhana.
- Detail rendering masih eksperimental: kompatibilitas OpenGL, bukan renderer produksi modern.

Dokumen teknis untuk implementasi terakhir: [Native viewport, world streaming, LOD, dan shading](docs/NATIVE-VIEWPORT-LOD.md).

### Fondasi engine yang sudah tersedia

- C++20/CMake project scaffold dan validator.
- Data-only mod package bootstrap serta resolver dependensi lokal.
- Scene graph, hierarchical transforms, serialisasi scene berversi, dan validasi.
- Registry asset, validasi referensi/dependensi, fingerprint SHA-256, serta importer OBJ dengan UV, normal, triangulasi, dan bounds.
- Parser teks MTL untuk properti material dasar; hubungan OBJ `mtllib/usemtl`, texture import, dan material binding penuh belum selesai.
- CPU reference rasterizer dengan depth buffer dan directional shading untuk output PPM.
- World-cell lifecycle model dan validator manifest `.ecworld`; jalur viewport streaming adalah lapisan awal, belum sistem world streaming produksi.
- Camera/projection math, input abstraction, bounded fixed-step frame clock, engine frame loop.
- Query collision AABB/sphere dan software PCM mixer; physics simulation dan audio-device output belum tersedia.
- CI build dan automated tests lintas Linux, Windows, dan macOS.

## Roadmap prioritas

1. **Stabilkan jalur viewport dan world LOD:** verifikasi build lintas platform, bounds proxy, shading, culling, dan fallback rendering.
2. **Material dan lighting:** mesh normals yang benar, material assignment, tekstur, pencahayaan, lalu pipeline PBR.
3. **World runtime produksi:** asynchronous I/O, worker scheduling, cancellation, eviction, floating origin, streaming telemetry, dan budget.
4. **Workflow editor:** asset browser/import UI, material editor, transform/gizmo workflow, save/recovery, undo/redo yang tahan lama.
5. **Runtime gameplay:** physics simulation, audio backend, scripting sandbox, play-in-editor, debugging/profiling.
6. **Pipeline konten dan mod SDK:** format berversi, validasi, permission model, compatibility checks, dan tooling bagi developer eksternal.
7. **Skala dan kualitas AAA:** stress tests, profiling target PC, rendering modern, kualitas aset, stabilitas, dan game contoh end-to-end.

Roadmap detail tunggal: [ROADMAP.md](ROADMAP.md).

## Modding

ECOS Studio ditujukan untuk game ECOS dan game pihak ketiga. Mod saat ini tetap data-only; belum ada eksekusi kode mod, download marketplace, atau runtime plugin tanpa sandbox. Lihat [Mod SDK bootstrap](modding/MOD-SDK.md).

## Build dan verifikasi

Membutuhkan CMake 3.20+ dan compiler C++20.

```sh
cmake -S ecos-studio -B build
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

Jalankan command sesuai target yang dibangun. Status CI terkini harus diperiksa pada workflow GitHub; dokumentasi ini tidak mengasumsikan semua job selalu hijau.

## Batas yang belum tercapai

Belum ada klaim bahwa ECOS Studio sudah setara Unreal Engine atau mampu memproduksi game AAA secara penuh. Renderer modern dengan PBR/texture pipeline, material editor lengkap, asynchronous world streaming produksi, physics simulation, scripting/gameplay runtime, play-in-editor, profiling suite, dan workflow distribusi matang masih menjadi pekerjaan terbuka.

## Prinsip engineering

- Pertahankan fitur yang ada; perubahan harus menambah kemampuan dan menjaga regresi.
- Jangan menulis ulang subsystem stabil tanpa kebutuhan teknis dan verifikasi.
- Jadikan CI penjaga regresi, bukan satu-satunya ukuran kemajuan.
- Sinkronkan source, tests, dan Markdown dalam perubahan yang sama.
- Gunakan Unreal Engine sebagai target kemampuan produk, bukan sumber kode untuk disalin.
