# Struktur Repository ECOS

**Tujuan:** satu peta resmi lokasi file agar pengembangan tidak kembali berputar pada penulisan lore.

## Prinsip
1. Root hanya memuat titik masuk, kebijakan repository, dan aset yang belum diklasifikasikan.
2. Seluruh lore aktif dan arsip berada di satu direktori: `lore-final/`.
3. `lore-final/README.md` adalah indeks lore; `00-KANON-FINAL-DAN-KONTINUITAS.md` adalah ringkasan lintas-domain.
4. Arsip di `lore-final/99-arsip/` bersifat historis, bukan sumber kanon aktif.
5. Jangan membuat direktori baru atau memindahkan aset hanya demi kerapian. Setiap direktori harus punya fungsi dan pemilik yang jelas.
6. Lore, spesifikasi teknis, kode sumber, aset game, build, dan bukti pengujian adalah jenis artefak berbeda. Jangan menaruh kode atau aset produksi di folder lore.

## Struktur saat ini
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

## Fungsi domain
| Direktori | Fungsi |
|---|---|
| `01-visi-dan-narasi/` | Premis, genre, protagonis, pemicu, tujuan, konflik, alur, dan aturan kesinambungan. |
| `02-dunia-dan-geografi/` | Kanon negara, wilayah, kota, bioma, transportasi, dan peta konseptual. |
| `03-gameplay-dan-aktivitas/` | Ruang lingkup aktivitas dan desain pengalaman pemain; belum berarti mekanik terimplementasi. |
| `04-karakter-dan-komunitas/` | Karakter pendukung dan prinsip hubungan sosial. |
| `05-ekonomi-dan-pekerjaan/` | Pekerjaan, kepemilikan, usaha, dan kerangka ekonomi dunia. |
| `06-teknologi-dan-sistem/` | Prinsip teknologi fiksional yang relevan bagi dunia. |
| `07-integrasi-proyek/` | Catatan hubungan dengan proyek eksternal hanya jika relevan dan dapat diverifikasi; bukan tempat mengklaim integrasi tanpa bukti. |
| `08-misteri-dan-legenda/` | Legenda, rumor, petunjuk, dan status verifikasinya. |
| `09-cerita/` | Struktur cerita dan cabang ending. |
| `90-audit-dan-keputusan/` | Catatan audit, keputusan lintas-domain, dan tindak lanjut. |
| `99-arsip/` | Dokumen lama yang dipertahankan untuk sejarah. |

## Aturan lokasi berikutnya
- **Kode sumber game/engine:** buat direktori sumber terpisah setelah engine dan bahasa implementasi dipilih; jangan menaruhnya di `lore-final/`.
- **Aset produksi:** klasifikasikan dua gambar root sebelum memindahkannya. Sampai kegunaannya teridentifikasi, keduanya tetap di root dan tidak boleh dihapus.
- **Dokumentasi teknis:** saat implementasi dimulai, buat `docs/` hanya jika ada dokumentasi teknis non-lore yang benar-benar dibutuhkan.
- **Pengujian dan CI:** konfigurasi build, test, dan workflow harus mengikuti stack yang benar-benar dipilih, bukan placeholder.
- **Arsip:** dokumen lama yang sudah dipindahkan tidak dihapus; substansi yang masih relevan harus disalin secara selektif ke dokumen aktif, bukan menghidupkan kembali seluruh dokumen usang.

## Gerbang sebelum menambah struktur
Sebuah direktori baru layak dibuat bila: (1) artefak nyata akan ditempatkan di sana, (2) fungsinya berbeda dari direktori yang ada, dan (3) cara memverifikasinya jelas. Hindari membuat folder kosong, roadmap tanpa artefak, atau integrasi semu.

## Status audit struktur
- Semua file lore aktif dan arsip berada di `lore-final/`.
- Audit tautan relatif Markdown dilakukan pada 57 file repository yang terdaftar; tidak ditemukan tautan relatif yang rusak.
- Dua gambar root sengaja dipertahankan karena fungsi spesifiknya belum diketahui.
- Repository saat ini terutama berupa dokumentasi worldbuilding. Struktur produksi kode, aset, build, dan pengujian akan dibangun ketika pilihan teknis dan artefak nyata sudah ada.
