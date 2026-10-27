# Struktur Repository ECOS

**Tujuan:** menjadi peta resmi lokasi artefak ECOS dan memisahkan kanon lore, data dunia, spesifikasi sistem, kode game, aset, pengujian, serta dokumentasi teknis.

## Prinsip

1. `lore-final/` tetap menjadi sumber kanon lore aktif; jangan membuat salinan kanon kedua di `lore/`.
2. Lore, data dunia terstruktur, spesifikasi sistem, kode sumber, aset produksi, dan bukti pengujian adalah jenis artefak berbeda.
3. Struktur target di bawah adalah arah pengembangan. Keberadaan folder atau dokumen **tidak** berarti peta, sistem, atau fitur game sudah diimplementasikan.
4. Folder baru dibuat dengan README singkat atau artefak nyata agar Git dapat melacaknya dan tujuannya jelas.
5. Dua gambar di root dipertahankan sampai fungsinya dipastikan. Jangan hapus atau pindahkan tanpa klasifikasi.
6. Jangan mengarang koordinat, batas geometri, rute, atau data populasi yang belum diputuskan dalam kanon/desain.

## Struktur repository saat ini di `main`

```text
/
├── README.md
├── REPO-STRUCTURE.md
├── IMG-20261024-WA2809.jpg
├── IMG-20261024-WA4357.jpg
└── lore-final/
    ├── README.md
    ├── 00-KANON-FINAL-DAN-KONTINUITAS.md
    ├── 01-visi-dan-narasi/
    ├── 02-dunia-dan-geografi/
    ├── 03-gameplay-dan-aktivitas/
    ├── 04-karakter-dan-komunitas/
    ├── 05-ekonomi-dan-pekerjaan/
    ├── 06-teknologi-dan-sistem/
    ├── 07-integrasi-proyek/
    ├── 08-misteri-dan-legenda/
    ├── 09-cerita/
    ├── 90-audit-dan-keputusan/
    └── 99-arsip/
        ├── draf-teknologi/
        └── referensi-lama/
```

Bagian ini mendeskripsikan keadaan `main` sebelum kerangka struktur baru digabung. Lihat bagian berikut untuk struktur target yang diselaraskan.

## Struktur target yang diselaraskan

```text
/
├── README.md
├── REPO-STRUCTURE.md
├── IMG-20261024-WA2809.jpg
├── IMG-20261024-WA4357.jpg
├── lore-final/                       # Kanon dan arsip lore; sumber kebenaran naratif
├── world/
│   ├── README.md
│   ├── nation-1_7m-km2/
│   │   └── README.md                 # Data wilayah negara skala lore ~1,7 juta km²
│   └── offline-10k-ha/
│       ├── README.md                 # Area inti offline 10.000 ha = 100 km²
│       ├── masjid-1k-ha/
│       │   └── README.md             # Kawasan 1.000 ha = 10 km² di dalam area offline
│       ├── juailin/
│       │   └── README.md             # Permukiman/lokasi Juailin
│       ├── buffer-zone-9k-ha/
│       │   └── README.md             # Sisa area 9.000 ha = 90 km²
│       └── systems-preview/
│           └── README.md             # Titik integrasi sistem area offline
├── systems/
│   ├── README.md
│   ├── government/
│   │   └── README.md
│   ├── transport-full/
│   │   └── README.md
│   └── biomes-full/
│       └── README.md
├── src/
│   └── README.md                     # Kode game setelah engine dipilih
├── assets/
│   └── README.md                     # Aset terklasifikasi; gambar root belum dipindah
├── tests/
│   └── README.md                     # Pengujian otomatis saat artefak dapat diuji tersedia
└── docs/
    └── README.md                     # Dokumentasi teknis non-lore
```

**Catatan status:** struktur target ini sedang disiapkan melalui PR #2. Sampai PR tersebut digabung, folder-folder baru di atas belum dianggap bagian dari `main`. README penanda hanya membuat folder terlacak; belum ada geometri peta final, kode game, engine terpilih, atau fitur yang dapat diklaim selesai.

## Fungsi domain lore

| Direktori | Fungsi |
|---|---|
| `lore-final/01-visi-dan-narasi/` | Premis, genre, protagonis, pemicu, tujuan, konflik, alur, dan aturan kesinambungan. |
| `lore-final/02-dunia-dan-geografi/` | Kanon negara, wilayah, kota, bioma, transportasi, dan peta konseptual. |
| `lore-final/03-gameplay-dan-aktivitas/` | Ruang lingkup aktivitas dan desain pengalaman pemain; bukan bukti mekanik sudah terimplementasi. |
| `lore-final/04-karakter-dan-komunitas/` | Karakter pendukung dan prinsip hubungan sosial. |
| `lore-final/05-ekonomi-dan-pekerjaan/` | Pekerjaan, kepemilikan, usaha, dan kerangka ekonomi dunia. |
| `lore-final/06-teknologi-dan-sistem/` | Prinsip teknologi fiksional yang relevan bagi dunia. |
| `lore-final/07-integrasi-proyek/` | Catatan hubungan dengan proyek eksternal hanya jika relevan dan dapat diverifikasi. |
| `lore-final/08-misteri-dan-legenda/` | Legenda, rumor, petunjuk, dan status verifikasinya. |
| `lore-final/09-cerita/` | Struktur cerita dan cabang ending. |
| `lore-final/90-audit-dan-keputusan/` | Catatan audit, keputusan lintas-domain, dan tindak lanjut. |
| `lore-final/99-arsip/` | Dokumen lama untuk kebutuhan sejarah; bukan sumber kanon aktif. |

## Aturan konsistensi dunia

- Skala negara ECOS sekitar 1,7 juta km² adalah skala lore nasional, bukan klaim seluruh wilayah sudah dapat dimainkan.
- Area inti offline adalah 10.000 ha (100 km²).
- Kawasan masjid 1.000 ha (10 km²) berada **di dalam** area offline dan di Provinsi Arunika; bukan wilayah administratif terpisah.
- Area sisa di luar kawasan masjid berjumlah 9.000 ha (90 km²). Pembagian geometri final belum ditetapkan.
- Jangan membuat poligon, koordinat, jaringan jalan, atau pembagian bioma seolah final sebelum dirancang dan divalidasi.

## Aturan lokasi artefak berikutnya

- **Data dunia:** `world/`; data mesin seperti JSON/GeoJSON ditambahkan saat skema dan geometri sudah ditetapkan.
- **Spesifikasi sistem:** `systems/`; bedakan rancangan dari implementasi.
- **Kode game/engine:** `src/`; pilih engine sebelum menambahkan kode spesifik engine.
- **Aset:** `assets/`; klasifikasikan gambar root sebelum memindahkan atau menghapusnya.
- **Pengujian:** `tests/`; tambahkan pengujian yang memeriksa artefak nyata, misalnya luas, containment, referensi lokasi, atau perilaku game.
- **Dokumentasi teknis:** `docs/`; jangan menduplikasi kanon lore.
- **Arsip:** jangan menghapus dokumen lama hanya karena struktur berubah; ambil substansi relevan secara selektif ke dokumen aktif.

## Status audit struktur

- Kanon aktif dan arsip lore berada di `lore-final/`.
- Dua gambar root sengaja dipertahankan karena fungsi spesifiknya belum diketahui.
- Struktur target memisahkan lore, data dunia, spesifikasi sistem, kode, aset, pengujian, dan dokumentasi.
- Repository masih terutama berupa dokumentasi worldbuilding. Scaffold direktori bukan game yang sudah berjalan; implementasi dan verifikasi harus dilaporkan terpisah.
