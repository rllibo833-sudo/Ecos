# Struktur Repository ECOS

## Prinsip
1. `lore-final/` adalah sumber kanon naratif aktif.
2. Lore aktif, arsip, data dunia, dan dokumentasi pengembangan harus dibedakan.
3. Konsep teknologi fiksional ECOS berada di domain lore aktif, bukan hanya di arsip historis.
4. Jangan mengubah ide terbuka menjadi fakta tanpa keputusan yang tercatat.
5. Jangan mengklaim sistem nyata terintegrasi atau fitur game berjalan tanpa bukti.
6. Engine game tidak termasuk ruang lingkup aktif saat ini.

## Struktur utama
```text
/
├── README.md
├── REPO-STRUCTURE.md
├── lore-final/
│   ├── 00-KANON-FINAL-DAN-KONTINUITAS.md
│   ├── ROADMAP-WORLDBUILDING.md
│   ├── 01-visi-dan-narasi/
│   ├── 02-dunia-dan-geografi/
│   ├── 03-gameplay-dan-aktivitas/
│   ├── 04-karakter-dan-komunitas/
│   ├── 05-ekonomi-dan-pekerjaan/
│   ├── 06-teknologi-dan-sistem/
│   ├── 07-integrasi-proyek/
│   ├── 08-misteri-dan-legenda/
│   ├── 09-cerita/
│   ├── 90-audit-dan-keputusan/
│   └── 99-arsip/
├── world/          # Kerangka data dunia; bukan peta final
├── systems/        # Kerangka spesifikasi domain; bukan implementasi
├── assets/         # Aset yang belum seluruhnya diklasifikasikan
├── docs/           # Dokumentasi proses dan keputusan non-kanon
└── tests/          # Pemeriksaan artefak saat data terstruktur tersedia
```

## Aturan per folder
- `lore-final/01`–`09`: keputusan naratif dan domain dunia.
- `lore-final/90`: audit, konflik kanon, dan keputusan yang masih perlu dibuat.
- `lore-final/99`: arsip historis; berguna sebagai sumber, tetapi bukan otoritas kanon.
- `world/`: kelak menyimpan skema/data peta setelah geometri dan format disepakati.
- `systems/`: spesifikasi domain dan hubungan antar-sistem, bukan kode game.
- `assets/`: gambar dan referensi visual; asal serta tujuan aset harus jelas.
- `docs/`: proses pengembangan dan status kerja.
- `tests/`: validasi konsistensi yang benar-benar dapat dijalankan terhadap artefak.

## Kebijakan branch
Satu branch utama harus cukup untuk membaca kanon terbaru. Branch eksperimen tidak menjadi sumber lore hanya karena memiliki kode atau commit tambahan. Sebelum branch lain dihapus oleh pemilik repo, informasi non-Godot yang unik harus dipindahkan ke main dan diperiksa. Riwayat Git lama tetap dapat menjadi arsip, tetapi bukan sumber keputusan harian.
