# Arsip historis: RANCANGAN-STRUKTUR-DOKUMENTASI-ECOS-V1.md

> Dipindahkan dari `referensi/RANCANGAN-STRUKTUR-DOKUMENTASI-ECOS-V1.md` saat struktur repo ECOS dirapikan. Dokumen ini dipertahankan untuk jejak historis dan bahan pembanding, bukan sebagai sumber kanon aktif. Jika bertentangan dengan kanon aktif di `lore/01` dan `lore/02`, ikuti kanon aktif.

---

# Rancangan Struktur Dokumentasi ECOS v1.0

**Status:** RANCANGAN — untuk ditinjau  
**Prioritas:** dunia, geografi, dan masterplan  
**Ruang lingkup:** dokumentasi konsep saja; bukan implementasi game.

> Dokumen ini adalah proposal struktur dokumentasi. Ia tidak menyatakan bahwa semua dokumen di bawah sudah dibuat, diverifikasi, disetujui, atau diimplementasikan.

## 1. Tujuan dan prinsip

ECOS menyatukan gagasan menjadi satu konsep game yang koheren; ini bukan penggabungan kode dari repo-repo sumber. Dokumentasi harus membuat dunia mudah dipahami, ditinjau, dan kelak dipresentasikan kepada studio game.

Prinsip wajib:
1. Dunia dan wilayah saling terhubung secara geografis dan fungsional.
2. Kawasan masjid merupakan bagian penting dari dunia dan kehidupan komunitas.
3. Kebebasan pemain dan beragam gaya bermain merupakan prinsip desain.
4. Empat bioma utama: tropis, pegunungan, pesisir, dan gurun.
5. 25 legenda dilacak sebagai lore yang berpotensi terhubung dengan tempat, karakter, dan eksplorasi.
6. Sumber gagasan, status verifikasi, asumsi, dan keputusan harus bisa ditelusuri.
7. Tidak memilih engine atau memulai coding dalam tahap dokumentasi ini.

## 2. Struktur folder yang diusulkan

Struktur berikut adalah target yang diusulkan, bukan daftar file yang sudah ada.

```text
referensi/
├── README.md
├── 00-visi-dan-identitas/
│   ├── 00-visi-ecos.md
│   ├── 01-identitas-dan-pilar-game.md
│   ├── 02-pengalaman-yang-ditawarkan.md
│   ├── 03-prinsip-kebebasan-pemain.md
│   └── 04-batasan-dan-asumsi-desain.md
├── 01-worldbuilding/
│   ├── 00-gambaran-umum-dunia.md
│   ├── 01-geografi-dan-bentuk-dunia.md
│   ├── 02-hubungan-antarwilayah.md
│   ├── 03-skala-dan-jarak-perjalanan.md
│   ├── 04-ekologi-dan-lingkungan.md
│   ├── 05-iklim-cuaca-dan-musim.md
│   ├── 06-sumber-daya-alam.md
│   └── 07-konsistensi-dunia.md
├── 02-bioma-dan-lingkungan/
│   ├── 00-kerangka-empat-bioma.md
│   ├── 01-bioma-tropis.md
│   ├── 02-bioma-pegunungan.md
│   ├── 03-bioma-pesisir.md
│   ├── 04-bioma-gurun.md
│   ├── 05-transisi-antarbioma.md
│   ├── 06-flora-fauna-dan-ekosistem.md
│   └── 07-ancaman-dan-peluang-lingkungan.md
├── 03-masterplan-dunia/
│   ├── 00-masterplan-ecos.md
│   ├── 01-pembagian-zona-dan-luas.md
│   ├── 02-kawasan-masjid-dan-pusat-peradaban.md
│   ├── 03-kota-dan-pusat-permukiman.md
│   ├── 04-desa-dan-komunitas-lokal.md
│   ├── 05-kawasan-pertanian-dan-pangan.md
│   ├── 06-kawasan-alam-dan-konservasi.md
│   ├── 07-kawasan-eksplorasi-dan-penemuan.md
│   ├── 08-jalur-transportasi-dan-konektivitas.md
│   ├── 09-sungai-pantai-dan-hidrologi.md
│   ├── 10-titik-penting-dan-landmark.md
│   └── 11-konsistensi-spasial-masterplan.md
├── 04-kawasan-masjid/
│   ├── 00-visi-kawasan-masjid.md
│   ├── 01-fungsi-dan-identitas-kawasan.md
│   ├── 02-zona-inti-dan-zona-pendukung.md
│   ├── 03-pendidikan-ilmu-dan-kebudayaan.md
│   ├── 04-pelayanan-masyarakat.md
│   ├── 05-perdagangan-dan-aktivitas-ekonomi.md
│   ├── 06-ruang-publik-dan-interaksi-sosial.md
│   ├── 07-arsitektur-dan-lanskap.md
│   ├── 08-hubungan-dengan-wilayah-lain.md
│   └── 09-aktivitas-pemain-di-kawasan.md
├── 05-lore-dan-25-legenda/
│   ├── 00-kerangka-lore-ecos.md
│   ├── 01-sejarah-dunia.md
│   ├── 02-prinsip-desain-25-legenda.md
│   ├── 03-indeks-25-legenda.md
│   ├── 04-pemetaan-legenda-ke-wilayah.md
│   ├── 05-legenda-dan-eksplorasi.md
│   ├── 06-legenda-dan-sejarah-lokal.md
│   ├── 07-legenda-dan-karakter.md
│   └── 08-status-verifikasi-legenda.md
├── 06-gameplay-dan-kebebasan/
│   ├── 00-pilar-gameplay.md
│   ├── 01-kebebasan-dan-pilihan-pemain.md
│   ├── 02-gaya-bermain-dan-jalur-progresi.md
│   ├── 03-eksplorasi-dan-penemuan.md
│   ├── 04-kehidupan-sehari-hari.md
│   ├── 05-membangun-dan-mengelola.md
│   ├── 06-pertanian-dan-pangan.md
│   ├── 07-ekonomi-dan-perdagangan.md
│   ├── 08-interaksi-sosial.md
│   ├── 09-aktivitas-dan-konsekuensi-pilihan.md
│   └── 10-peta-aktivitas-berdasarkan-wilayah.md
├── 07-karakter-dan-komunitas/
│   ├── 00-kerangka-karakter-dan-npc.md
│   ├── 01-komunitas-dan-kehidupan-lokal.md
│   ├── 02-peran-dan-profesi.md
│   ├── 03-hubungan-sosial.md
│   └── 04-karakter-penting-dan-penghubung-lore.md
├── 08-arah-visual-dan-audio/
│   ├── 00-identitas-visual-ecos.md
│   ├── 01-referensi-lingkungan.md
│   ├── 02-arsitektur-dan-permukiman.md
│   ├── 03-arah-visual-empat-bioma.md
│   ├── 04-arah-visual-kawasan-masjid.md
│   ├── 05-desain-landmark-dan-legenda.md
│   └── 06-arah-audio-dan-atmosfer.md
├── 09-sistem-dunia-konseptual/
│   ├── 00-prinsip-simulasi-dunia.md
│   ├── 01-interaksi-lingkungan.md
│   ├── 02-ekologi-dan-sumber-daya.md
│   ├── 03-siklus-kehidupan-komunitas.md
│   ├── 04-keterkaitan-sistem-gameplay.md
│   └── 05-asumsi-dan-pertanyaan-terbuka.md
├── 10-paket-studio/
│   ├── 00-project-overview.md
│   ├── 01-ringkasan-pitch-ecos.md
│   ├── 02-game-design-document.md
│   ├── 03-world-and-level-design.md
│   ├── 04-world-art-bible.md
│   ├── 05-kebutuhan-produksi.md
│   └── 06-daftar-pertanyaan-untuk-studio.md
├── 11-produksi-dan-kriteria-kualitas/
│   ├── 00-prinsip-pengembangan-konsep.md
│   ├── 01-prioritas-dokumentasi.md
│   ├── 02-kriteria-penerimaan-dokumen.md
│   ├── 03-risiko-dan-ketergantungan.md
│   └── 04-catatan-keputusan-desain.md
└── 12-sumber-dan-keterlacakan/
    ├── 00-indeks-sumber.md
    ├── 01-pemetaan-konsep-antar-repo.md
    ├── 02-daftar-sumber-terverifikasi.md
    ├── 03-daftar-sumber-belum-terverifikasi.md
    ├── 04-matriks-keputusan-desain.md
    ├── 05-riwayat-perubahan-konsep.md
    └── 06-glosarium-ecos.md
```

## 3. Fungsi dan kriteria selesai per kelompok

| Kelompok | Fungsi utama | Hubungan utama | Status awal | Kriteria selesai |
|---|---|---|---|---|
| 00 Visi dan identitas | Menentukan identitas, pilar, dan pengalaman yang dituju | Semua kelompok | RANCANGAN | Visi jelas; pilar tidak bertentangan; asumsi dipisahkan dari keputusan |
| 01 Worldbuilding | Menjelaskan geografi, jarak, iklim, ekologi, dan sumber daya | Bioma dan masterplan | RANCANGAN | Wilayah dan hubungan geografis konsisten; pertanyaan terbuka dicatat |
| 02 Bioma | Menetapkan karakter empat bioma dan transisinya | Worldbuilding, masterplan, gameplay | RANCANGAN | Tiap bioma memiliki lanskap, ekologi, sumber daya, peluang, dan tantangan |
| 03 Masterplan | Menetapkan pembagian ruang dan hubungan zona | Worldbuilding, kawasan masjid, gameplay | RANCANGAN | Luas dapat diperiksa; akses, batas, dan hubungan zona konsisten |
| 04 Kawasan masjid | Merancang fungsi inti dan pendukung serta keterkaitannya dengan dunia | Masterplan, komunitas, gameplay, visual | RANCANGAN | Fungsi, zona, akses, aktivitas, dan hubungan regional dijelaskan |
| 05 Lore dan 25 legenda | Menautkan sejarah, legenda, lokasi, dan eksplorasi | Masterplan, karakter, gameplay, sumber | RANCANGAN | Indeks memuat 25 entri yang dapat dilacak; entri yang belum diketahui ditandai |
| 06 Gameplay dan kebebasan | Menentukan pilihan, aktivitas, progresi, dan konsekuensi | Masterplan, bioma, karakter, sistem dunia | RANCANGAN | Ada contoh gaya bermain berbeda, aktivitas opsional, dan dampak pilihan |
| 07 Karakter dan komunitas | Menjelaskan penghuni dunia, peran, dan hubungan sosial | Lore, permukiman, gameplay | RANCANGAN | Peran komunitas dan hubungan dengan dunia dapat dipahami |
| 08 Arah visual dan audio | Menjaga identitas visual dan atmosfer konsisten | Bioma, masjid, landmark, paket studio | RANCANGAN | Referensi dan aturan arah seni terdokumentasi; bukan klaim aset sudah dibuat |
| 09 Sistem dunia konseptual | Menjelaskan hubungan konseptual ekologi, komunitas, dan gameplay | Worldbuilding dan gameplay | RANCANGAN | Sistem, masukan, dampak, asumsi, dan pertanyaan terbuka dijelaskan tanpa implementasi |
| 10 Paket studio | Menyediakan ringkasan untuk calon studio atau mitra | Dokumen yang sudah ditinjau | RANCANGAN | Ringkasan konsisten dengan dokumen sumber dan membedakan fakta dari usulan |
| 11 Produksi dan kualitas | Mengatur prioritas, risiko, dan pemeriksaan dokumen | Semua kelompok | RANCANGAN | Kriteria penerimaan dan ketergantungan terdokumentasi |
| 12 Sumber dan keterlacakan | Melacak asal ide, status verifikasi, keputusan, dan istilah | Semua kelompok | RANCANGAN | Klaim sumber memiliki bukti atau ditandai belum diverifikasi |

### Kriteria umum setiap dokumen

Dokumen baru bisa diajukan untuk persetujuan apabila tujuan dan ruang lingkupnya jelas, istilah konsisten, hubungan dengan dokumen lain tercatat, sumber dan asumsi ditandai, tidak bertentangan dengan keputusan yang sudah disetujui, serta kriteria khususnya terpenuhi. Panjang dokumen atau jumlah file bukan ukuran keberhasilan.

## 4. Hubungan antardokumen

```text
VISI DAN IDENTITAS
        ↓
GAMBARAN UMUM DUNIA
        ↓
GEOGRAFI DAN LINGKUNGAN ↔ EMPAT BIOMA
        ↓
MASTERPLAN DUNIA
        ├── Kawasan masjid dan pusat peradaban
        ├── Kota, desa, dan komunitas
        ├── Alam, sumber daya, dan eksplorasi
        └── Transportasi dan konektivitas
        ↓
LORE DAN 25 LEGENDA
        ↓
GAMEPLAY DAN KEBEBASAN PEMAIN
        ↓
SISTEM DUNIA KONSEPTUAL
        ↓
PAKET STUDIO

SUMBER DAN KETERLACAKAN berlaku untuk semua kelompok.
```

Contoh: jika sebuah legenda dikaitkan dengan pegunungan, sumber dan status legendanya dicatat; dokumen bioma menjelaskan lingkungan; masterplan menunjukkan lokasi konseptual; gameplay menerangkan potensi pengalaman pemain. Dengan begitu, hubungan antaride dapat ditelusuri dan tidak bergantung pada ingatan.

## 5. Status dokumen

| Status | Arti |
|---|---|
| `RANCANGAN` | Struktur atau tujuan masih berupa proposal |
| `PERLU-VERIFIKASI` | Sumber atau fakta belum diperiksa memadai |
| `DRAF` | Isi awal tersedia dan menunggu tinjauan |
| `MENUNGGU-PERSETUJUAN` | Draf siap diputuskan |
| `DISETUJUI` | Isi telah disetujui sebagai keputusan desain |
| `SIAP-UNTUK-PRODUKSI` | Spesifikasi cukup rinci untuk tim produksi; bukan bukti implementasi |
| `DIGANTIKAN` | Isi telah diganti dan bukan lagi acuan aktif |

Semua kelompok dalam rancangan ini berstatus awal `RANCANGAN`. Status file yang sudah ada tidak boleh diubah tanpa pemeriksaan.

## 6. Aturan khusus untuk unsur wajib

### 25 legenda
- Jangan mengarang nama, sejarah, atau atribut untuk mengisi kekosongan.
- Pertahankan indeks dengan 25 slot yang bisa ditelusuri.
- Tandai tiap entri: terverifikasi, sebagian terverifikasi, atau belum terverifikasi.
- Hubungkan legenda dengan lokasi dan gameplay hanya jika ada dasar desain yang jelas.

### Kawasan masjid
- Perlakukan luas 10.000 hektare sebagai asumsi masterplan game sesuai arahan konsep, bukan survei atau rencana pembangunan nyata.
- Tegaskan apakah angka tersebut mencakup seluruh dunia permainan atau kawasan khusus sebelum pembagian luas disahkan.
- Periksa jumlah seluruh alokasi zona sebelum masterplan dianggap konsisten.

### Kebebasan pemain
- Jelaskan aktivitas opsional dan pilihan gaya bermain.
- Jangan menyebut dunia “bebas” hanya sebagai slogan; beri contoh tindakan dan konsekuensi.
- Pastikan tiap wilayah memiliki alasan untuk dijelajahi atau dihuni.

### Empat bioma
- Tropis, pegunungan, pesisir, dan gurun masing-masing harus memiliki identitas, ekologi, sumber daya, tantangan, dan aktivitas.
- Jelaskan transisi dan konektivitas antarbioma agar dunia tidak terasa seperti kumpulan peta terpisah.

### Sumber
- Bedakan ide asli ECOS, konsep yang diadaptasi, sumber eksternal, asumsi, dan keputusan yang telah disetujui.
- Jangan mengklaim isi repo LynOS, PusingCuk, Mnemos-RLI, Steadfast, atau file lain telah diverifikasi tanpa memeriksanya.
- Catat sumber, lokasi/bagian yang relevan, status pemeriksaan, dan hubungan dengan keputusan desain.

## 7. Prioritas pengerjaan setelah struktur disetujui

| Urutan | Fokus | Hasil yang diharapkan |
|---|---|---|
| P0 | Inventaris dokumen dan sumber yang sudah ada | Mengetahui bahan yang tersedia dan kekosongan tanpa menebak |
| P1 | Visi, gambaran umum dunia, dan geografi | Fondasi dunia konsisten |
| P2 | Empat bioma dan lingkungan | Identitas dan transisi bioma |
| P3 | Masterplan dan pembagian zona | Susunan ruang yang dapat ditinjau |
| P4 | Kawasan masjid dan permukiman | Fungsi kawasan dan hubungan regional |
| P5 | Lore dan 25 legenda | Cerita yang terhubung ke dunia dan sumber |
| P6 | Gameplay dan kebebasan pemain | Aktivitas, pilihan, dan konsekuensi |
| P7 | Sistem dunia konseptual | Keterkaitan sistem pada level desain |
| P8 | Paket studio | Ringkasan dari konsep yang sudah ditinjau |

## 8. Pertanyaan terbuka yang harus dicatat

1. Apakah 10.000 hektare merupakan keseluruhan area permainan atau kawasan utama di dalam dunia yang lebih luas?
2. Bagaimana empat bioma terhubung secara geografis?
3. Seberapa jauh jarak antarkota, desa, dan landmark?
4. Berapa luas kawasan inti masjid dan zona pendukungnya?
5. Aktivitas mana yang opsional, dan sejauh mana pemain dapat mengubah lingkungan?
6. Apa sumber dan status masing-masing dari 25 legenda?
7. Gagasan mana dari repo sumber yang benar-benar ingin dipertahankan di ECOS?

Pertanyaan tersebut tidak perlu dijawab sebelum struktur dokumentasi disetujui, tetapi harus tetap terlihat dan tidak dianggap sudah diputuskan.

## 9. Urutan penerapan dokumentasi

1. Tinjau struktur ini.
2. Periksa dokumen yang sudah ada di repo dan petakan ke struktur tujuan.
3. Tandai dokumen yang dapat dipertahankan, perlu direvisi, perlu dipindahkan, atau belum tersedia.
4. Susun dokumen prioritas dan ajukan untuk ditinjau.
5. Setelah isinya disetujui, siapkan paket studio berdasarkan sumber yang konsisten.

Tidak ada coding, pemilihan engine, atau klaim implementasi game dalam tahap ini.

## 10. Checklist penerimaan rancangan

- [x] Struktur profesional dan paket studio tercakup.
- [x] Dunia, geografi, bioma, dan masterplan menjadi prioritas.
- [x] Kawasan masjid memiliki kelompok dokumentasi khusus.
- [x] 25 legenda memiliki indeks dan status verifikasi.
- [x] Kebebasan pemain menjadi prinsip desain eksplisit.
- [x] Sumber dan asal konsep dapat ditelusuri.
- [x] Fungsi, hubungan, status awal, dan kriteria selesai per kelompok ditetapkan.
- [x] Pertanyaan yang belum diputuskan ditandai.
- [x] Rancangan tidak mengklaim semua file sudah ada atau terverifikasi.
- [x] Tidak memilih engine dan tidak memulai coding.

---

**Keputusan yang diminta dari pemilik proyek:** tinjau struktur dan aturan ini. Setelah disetujui, langkah berikutnya adalah inventaris serta pemetaan dokumen yang sudah ada—bukan langsung membuat seluruh file kosong.
