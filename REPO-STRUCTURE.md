# Struktur Repository ECOS

## Sumber kebenaran

- `ecos-studio/README.md`: ringkasan produk dan kemampuan aktual.
- `ecos-studio/ARCHITECTURE.md`: batas subsystem dan arah arsitektur.
- `ecos-studio/ROADMAP.md`: satu roadmap engineering kanonis.
- `ecos-studio/docs/`: spesifikasi teknis per subsystem; indeks berada di `ecos-studio/docs/README.md`.
- `lore-final/`: kanon naratif ECOS, terpisah dari status implementasi engine.
- Riwayat Git: arsip perubahan; bukan pengganti dokumentasi status terkini.

## Struktur utama

```text
/
├── README.md
├── REPO-STRUCTURE.md
├── ecos-studio/
│   ├── README.md
│   ├── ARCHITECTURE.md
│   ├── ROADMAP.md
│   ├── src/ and include/       # implementasi C++20
│   ├── tests/                  # automated regression tests
│   ├── examples/               # contoh yang harus cocok dengan format aktif
│   ├── docs/                   # dokumentasi teknis kanonis
│   └── modding/                # kontrak mod SDK saat ini
├── lore-final/                 # kanon naratif aktif dan arsip lore
├── world/                      # kerangka data dunia, bukan bukti runtime
├── systems/                    # spesifikasi domain
├── assets/                     # aset dan referensi
├── docs/                       # dokumentasi repository umum
└── tests/                      # validasi artefak repository
```

## Kebijakan dokumentasi

1. Tandai kemampuan sebagai **Implemented**, **Partial/Experimental**, atau **Planned** berdasarkan source dan bukti yang ada.
2. Ketika source berubah, perbarui README/arsitektur/roadmap atau dokumen subsystem terkait pada commit yang sama.
3. Jangan mempertahankan kalimat lama seperti “GPU renderer planned” jika viewport GPU sudah ada; jelaskan batas aktualnya.
4. Jangan menghapus spesifikasi subsystem yang masih akurat hanya karena kecil. Perbarui dokumen yang kedaluwarsa; pindahkan hanya draf atau rencana yang benar-benar digantikan ke direktori arsip dengan alasan dan tautan ke dokumen kanonis.
5. Lore, target visi, dan roadmap bukan bukti bahwa fitur sudah berjalan.
6. Jangan menyatakan setara Unreal Engine atau siap AAA sampai workflow produksi, kualitas visual, performa, stabilitas, dan kompatibilitas terbukti.
