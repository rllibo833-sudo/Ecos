# ECOS — Game, Studio, dan Dunia

ECOS mengembangkan dua produk yang saling terhubung:

- **ECOS Studio** — engine, editor, dan toolkit modding general-purpose untuk developer eksternal dan pembuat game.
- **ECOS** — game open-world unggulan dengan visi dunia fiksi sekitar **1,7 juta km²**, sebagai salah satu konsumen teknologi ECOS Studio.

## Status engineering

Branch `ecos-studio` sudah melewati tahap CLI-only. Saat ini source mencakup fondasi C++20, scene graph dan format scene, import mesh OBJ, parsing material MTL dasar, CPU reference renderer, native desktop viewport berbasis GLFW/OpenGL, World Outliner/Inspector dasar, serta awal viewport world-cell streaming dan LOD proxy.

Dokumentasi status dan batas implementasi yang menjadi rujukan utama:

- [ECOS Studio — kemampuan dan batas saat ini](ecos-studio/README.md)
- [Arsitektur engine](ecos-studio/ARCHITECTURE.md)
- [Roadmap engineering kanonis](ecos-studio/ROADMAP.md)
- [Native viewport, streaming, LOD, dan shading](ecos-studio/docs/NATIVE-VIEWPORT-LOD.md)
- [Indeks dokumentasi teknis](ecos-studio/docs/README.md)

**Visi:** membangun studio editor mandiri dengan kedalaman workflow dan target kemampuan kelas Unreal Engine, untuk membuat game berambisi AAA. Ini adalah arah produk, bukan klaim bahwa ECOS Studio saat ini sudah setara UE atau siap produksi AAA. Setiap kemampuan ditandai sesuai bukti implementasi aktual.

## Lore dan kanon ECOS

- [Kanon dan kontinuitas](lore-final/00-KANON-FINAL-DAN-KONTINUITAS.md)
- [Kanon induk dunia](lore-final/02-dunia-dan-geografi/00-kanon-induk-dunia-ecos.md)
- [Visi dan narasi](lore-final/01-visi-dan-narasi/01-premis-dunia.md)
- [Teknologi identitas ECOS](lore-final/06-teknologi-dan-sistem/README.md)
- [Roadmap worldbuilding](lore-final/ROADMAP-WORLDBUILDING.md)

## Prinsip repository

Lore, rencana produk, kode yang sudah diimplementasikan, dan hasil verifikasi adalah kategori berbeda. Markdown teknis harus mengikuti source terkini; roadmap tidak boleh menandai aspirasi sebagai fitur selesai. Dokumen lama yang bertentangan harus diperbarui atau dipindahkan ke arsip historis dengan penanda yang jelas, bukan dibiarkan seolah-olah menggambarkan engine saat ini.
