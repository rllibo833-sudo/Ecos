# Status Rekonstruksi Repository ECOS

**Tujuan:** mencatat kondisi repo setelah fokus dialihkan dari prototipe game ke pembangunan dunia.

## Temuan audit awal
- `main` memiliki lore terstruktur dan indeks kontinuitas, tetapi beberapa profil teknologi hanya tersimpan sebagai template kosong di arsip.
- Struktur world/systems/src berisi banyak README penanda, bukan data peta final atau sistem yang berjalan.
- Branch prototipe memiliki kode Godot dan artefak uji yang belum menjadi dasar kanon dunia.
- Dokumen kanon sendiri secara eksplisit menyatakan bahwa konsep dan implementasi harus dibedakan.
- Bab novel tidak diperlukan sebagai sumber kanon repo saat ini; naskah kerja berada di notebook penulis.

## Perubahan arah
- Fokus utama: lore, worldbuilding, atlas, masyarakat, sejarah, teknologi fiksional, dan kontinuitas narasi.
- Godot/prototipe dikeluarkan dari main untuk sementara.
- Profil teknologi inti dipindahkan dari sekadar daftar nama menjadi dokumen konsep aktif.
- Arsip historis tetap dipertahankan sebagai referensi, tetapi tidak lagi menjadi sumber keputusan utama.
- Branch eksperimen tidak menjadi sumber kanon aktif. Informasi unik yang relevan perlu dikonsolidasikan ke main sebelum branch dihapus.

## Status artefak
| Area | Status saat ini | Yang belum boleh diklaim |
|---|---|---|
| Lore dan kontinuitas | Ada baseline terstruktur | Belum berarti semua detail dunia telah final |
| Geografi dan atlas | Ada skala, provinsi, dan konsep wilayah | Belum ada peta final terukur |
| Masyarakat dan ekonomi | Ada aturan dasar | Belum ada model lengkap harga, upah, populasi, atau institusi |
| Teknologi fiksional | Profil awal aktif dibuat | Belum menjadi spesifikasi game atau integrasi teknis |
| Cerita utama | Ada struktur dan cabang akhir konseptual | Belum menjadi naskah lengkap atau misi terimplementasi |
| Novel | Bab 1 adalah naskah kerja penulis | Bukan sumber kanon otomatis untuk mengubah lore |
| Game | Tidak menjadi ruang lingkup aktif | Tidak ada klaim game playable, APK, atau engine tervalidasi |

## Gerbang sebelum menghapus branch lain
1. Bandingkan isi unik branch terhadap main.
2. Ambil hanya lore/data non-engine yang bermanfaat dan tidak bertentangan.
3. Jangan memindahkan kode atau workflow Godot ke main.
4. Periksa README, link internal, dan status dokumen setelah konsolidasi.
5. Setelah main mandiri dan terverifikasi, pemilik repo dapat menghapus branch eksperimen.

## Pertanyaan terbuka utama
- Sejarah politik dan kronologi negara ECOS.
- Geometri wilayah dan konektivitas antarwilayah.
- Populasi, mata uang, harga, upah, serta ekonomi lintas provinsi.
- Lembaga tempat Rezman bekerja dan kewenangan investigasinya.
- Identitas orang yang disukai Rezman dan bentuk kehilangan pemicu.
- Makna teknis “determinan” pada Cuk Reality Engine.
- Hubungan fungsional dan tata kelola antar-teknologi.
