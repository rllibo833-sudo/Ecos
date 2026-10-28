# ECOS — Game, Studio, dan Dunia

ECOS memiliki dua sasaran produk yang dibangun bersama:

1. **ECOS Studio** — engine dan studio game PC untuk developer, studio, dan modder; ditujukan menjadi produk komersial yang terus berkembang.
2. **ECOS** — game open-world PC unggulan dengan dunia negara fiksi sekitar 1,7 juta km², dibangun di atas teknologi ECOS Studio.

Branch ini mengawali fondasi source code ECOS Studio. Implementasi saat ini masih berupa C++20 command-line bootstrap untuk membuat dan memvalidasi scaffold proyek game serta paket mod data-only. Ini **belum** merupakan editor 3D, renderer, physics/audio runtime, atau game playable. Lihat [ECOS Studio README](ecos-studio/README.md), [arsitektur](ecos-studio/ARCHITECTURE.md), dan [roadmap](ecos-studio/ROADMAP.md).

## Lore dan kanon ECOS

- [Kanon dan kontinuitas](lore-final/00-KANON-FINAL-DAN-KONTINUITAS.md)
- [Kanon induk dunia](lore-final/02-dunia-dan-geografi/00-kanon-induk-dunia-ecos.md)
- [Visi dan narasi](lore-final/01-visi-dan-narasi/01-premis-dunia.md)
- [Teknologi identitas ECOS](lore-final/06-teknologi-dan-sistem/README.md)
- [Roadmap worldbuilding](lore-final/ROADMAP-WORLDBUILDING.md)

## Kanon ringkas

- ECOS: negara fiksi sekitar 1,7 juta km² dengan beberapa wilayah terpisah.
- Wilayah inti desain: 10.000 ha; kawasan masjid 1.000 ha berada di dalamnya.
- Empat provinsi kerja: Arunika, Wanasera, Tirta Nusa, Giriwana. Ibu kota nasional: Nawasena.
- Juailin adalah kota kecil asal Rezman, dengan arus informasi sangat cepat.
- Rezman adalah peternak dan pekebun yang kesulitan mengikuti arus informasi; ia menggunakan perangkat genggam dengan LynOS dan bekerja di bidang digital forensik.
- Living Shared Knowledge Graph menghubungkan informasi dan bukti, tetapi bukan sistem mahatahu.
- Bahasa .cuk, PusingCuk AI, Cuk Reality Engine, dan Mnemos-RLI adalah teknologi fiksional yang harus memiliki fungsi, batas, risiko, dan konsekuensi.
- ECOS bukan cyberpunk. Dunia harus terasa hidup, beragam, dan tidak sempurna.

## Prinsip pengembangan

Lore, spesifikasi teknis, dan implementasi nyata adalah tiga hal berbeda. Fitur engine hanya boleh disebut selesai jika kode implementasinya tersedia dan kemampuan tersebut benar-benar dibuktikan. ECOS Studio ditujukan untuk developer eksternal juga, sehingga format proyek, SDK, dokumentasi, dan keamanan mod menjadi bagian dari produk—bukan hanya kebutuhan internal game ECOS.
