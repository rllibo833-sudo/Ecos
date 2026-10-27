# Arsip historis: AUDIT-KONSEP-DUNIA-DAN-MASTERPLAN.md

> Dipindahkan dari `referensi/AUDIT-KONSEP-DUNIA-DAN-MASTERPLAN.md` saat struktur repo ECOS dirapikan. Dokumen ini dipertahankan untuk jejak historis dan bahan pembanding, bukan sebagai sumber kanon aktif. Jika bertentangan dengan kanon aktif di `lore/01` dan `lore/02`, ikuti kanon aktif.

---

# Audit Konsep Dunia dan Masterplan ECOS

- ID: ECOS-WORLD-AUDIT-001
- Status: DRAF
- Versi: 0.1
- Cakupan: konsistensi dokumen masterplan, skala dunia, empat bioma, kawasan masjid, dan sumber konsep.
- Batas: audit dokumentasi saja; tidak memilih engine, tidak membuat kode, dan tidak menyatakan game sudah dibangun.

## 1. Ringkasan eksekutif

ECOS sudah memiliki fondasi konseptual yang bisa dipakai: visi open-world life simulation, pusat peradaban yang berorientasi pada masjid, empat bioma, 13 alokasi zona, prinsip konektivitas, serta kriteria yang membedakan dokumen dari game yang benar-benar dapat dimainkan.

Namun, masterplan belum cukup konsisten untuk dianggap sebagai rancangan dunia yang matang. Masalah utamanya bukan kekurangan file, melainkan keputusan dasar yang belum diputuskan dan beberapa istilah yang tidak konsisten. Karena itu, dokumen ini tidak mengesahkan masterplan; dokumen ini mencatat apa yang bisa dipertahankan, apa yang perlu diperbaiki, dan keputusan apa yang harus dibuat sebelum rincian turunan ditulis.

**Rekomendasi utama:** jangan membuat puluhan dokumen turunan dahulu. Selesaikan keputusan dunia, geografi, dan skala; lalu perbarui masterplan induk dan buat dokumen geografi/bioma berdasarkan keputusan itu.

## 2. Bahan yang diperiksa

1. `referensi/MASTERPLAN-ECOS-10000-HEKTARE.md` — masterplan konseptual 10.000 ha, 13 zona, empat bioma, konektivitas, ekologi, dan 25 slot legenda.
2. `referensi/SPESIFIKASI-TEKNIS-MASTERPLAN.md` — sistem koordinat 10 km × 10 km, grid 20 × 20, layer peta, contoh skema, topologi dan tahapan produksi.
3. `referensi/RANCANGAN-STRUKTUR-DOKUMENTASI-ECOS-V1.md` — usulan struktur dokumen dan pertanyaan terbuka.
4. `referensi/STANDAR-KUALITAS-DOKUMENTASI-ECOS.md` — standar kualitas dan aturan anti-klaim palsu.
5. `rllibo833-sudo/kawasan-masjid-public/README.md` — sumber publik terkait kawasan masjid; README menyebut “Kawasan Masjid 1000 Hektare” dan secara eksplisit menandainya sebagai konsep/ideasi, bukan proyek yang dibangun atau disahkan.

Audit ini hanya menganggap isi yang terlihat pada dokumen tersebut sebagai bukti. Nama file atau repo lain tidak cukup untuk menyimpulkan apa isi atau statusnya. Isi 25 legenda belum diverifikasi satu per satu dalam audit ini.

## 3. Temuan prioritas

| ID | Prioritas | Temuan | Dampak | Tindakan yang disarankan |
|---|---|---|---|---|
| AUD-01 | P0 | Ada selisih skala 1.000 ha pada README kawasan masjid dan 10.000 ha pada masterplan ECOS. | Tidak jelas apakah 10.000 ha adalah seluruh dunia game, kawasan masjid, atau kawasan regional yang mengelilinginya. | Tetapkan satu definisi skala dan catat hubungan angka 1.000 ha dengan angka 10.000 ha. Jangan menyamakan keduanya tanpa keputusan. |
| AUD-02 | P0 | Empat bioma besar ditempatkan dalam area konseptual 100 km², tetapi belum ada rancangan elevasi, garis pantai, curah hujan, bayangan hujan, atau transisi iklim yang menjelaskan koeksistensinya. | Dunia berisiko terasa seperti empat peta tempelan, bukan satu wilayah yang koheren. | Buat model geografis fiktif yang eksplisit: bentuk daratan, pegunungan, arah aliran air, posisi pesisir, dan alasan wilayah kering terbentuk. |
| AUD-03 | P0 | Dokumen teknis menyebut layer “Districts — 12 distrik fungsional”, padahal masterplan memiliki 13 zona. | Struktur data dan dokumentasi dapat berbeda hitungan. | Ubah istilah menjadi 13 zona atau jelaskan secara eksplisit bila ada 12 distrik yang dikelompokkan dari 13 zona. Untuk saat ini, jangan menganggap pengelompokan itu sudah ada. |
| AUD-04 | P1 | Jalan dan utilitas Z06 serta koridor regional Z13 dapat tumpang tindih secara fisik dengan zona lain, sementara total luas diminta tidak dihitung ganda. | Alokasi 10.000 ha dapat terlihat tepat di tabel, tetapi tidak punya aturan geometri yang jelas. | Putuskan apakah alokasi adalah petak eksklusif atau kategori fungsi. Jika eksklusif, koridor harus punya poligon sendiri; jika kategori, jangan jumlahkan semua kategori sebagai luas lahan unik. |
| AUD-05 | P1 | Titik pusat contoh (5000, 5000, 0) di spesifikasi teknis hanyalah placeholder, tetapi mudah dibaca sebagai posisi final. | Risiko keputusan sementara dianggap sebagai keputusan desain. | Beri label placeholder pada semua koordinat contoh dan sediakan daftar keputusan koordinat yang belum ditetapkan. |
| AUD-06 | P1 | Kerangka 25 legenda hanya membagi 25 slot ke lima kelompok sementara; nama dan sumber belum diverifikasi. | Lore berisiko terisi oleh nama atau kisah karangan yang keliru dianggap berasal dari sumber. | Buat indeks LEG-01–LEG-25 dengan kolom sumber, status, isi yang benar-benar diketahui, dan kolom kosong yang jelas. |
| AUD-07 | P1 | “Kebebasan pemain” dinyatakan sebagai pilar, tetapi belum dijabarkan menjadi tindakan, pilihan, konsekuensi, dan batasan. | Open world berisiko hanya menjadi ukuran peta, bukan kebebasan yang terasa dalam permainan. | Tulis skenario gameplay konkret untuk beberapa gaya bermain sebelum menyusun daftar aktivitas besar. |
| AUD-08 | P1 | Spesifikasi teknis sudah membahas grid, layer, schema, streaming, dan fase produksi sebelum bentuk dunia serta kebutuhan pengalaman ditetapkan. | Detail teknis dapat mengunci asumsi terlalu dini dan mengalihkan fokus dari desain. | Pertahankan dokumen itu sebagai catatan teknis konseptual; jangan perlakukan grid 20×20 atau pilihan streaming sebagai keputusan final. |
| AUD-09 | P2 | Daftar zona menyebut tujuan umum, tetapi belum memuat batas, tetangga, pintu masuk, landmark, kepadatan, dan aktivitas utama per wilayah. | Pembaca belum dapat membayangkan perjalanan atau membandingkan distrik secara konsisten. | Setelah geografi dasar diputuskan, buat lembar spesifikasi per zona dengan struktur yang sama. |
| AUD-10 | P2 | Sumber README kawasan masjid menyatakan konsepnya belum dibangun/disahkan. | Materi visual dan data sumber tidak boleh diperlakukan sebagai survei atau rencana fisik terverifikasi. | Gunakan sebagai referensi ideasi saja, dan pertahankan label konseptual pada materi turunannya. |

## 4. Keputusan yang harus dikunci sebelum rincian turunan

### D-01 — Definisi skala 10.000 ha
Pilih dan dokumentasikan salah satu definisi:
- **Opsi A:** 10.000 ha adalah seluruh area dunia yang dapat dimainkan; kawasan masjid 1.000 ha menjadi bagian di dalamnya.
- **Opsi B:** 10.000 ha adalah wilayah regional yang lebih luas, sedangkan kawasan inti masjid memiliki luas tersendiri.
- **Opsi C:** angka 1.000 ha dari repo kawasan masjid adalah versi lama/alternatif dan 10.000 ha menggantikannya untuk konsep ECOS.

Tidak ada opsi yang ditetapkan oleh audit ini. Opsi yang dipilih harus menyebutkan batas dan hubungan kedua angka, bukan sekadar memilih angka terbesar.

### D-02 — Geografi yang mengikat empat bioma
Rancang satu model dunia dengan urutan hubungan yang masuk akal. Contoh pola untuk dievaluasi, bukan keputusan final:
- pegunungan sebagai daerah elevasi tinggi dan sumber sungai;
- hutan tropis dan pertanian di wilayah yang cukup mendapat air;
- sungai mengalir menuju dataran rendah dan pesisir;
- wilayah kering berada di balik penghalang pegunungan atau dalam cekungan dengan curah hujan rendah;
- garis pantai membentuk batas alami dan titik perdagangan.

Pola tersebut harus disesuaikan dengan bentuk dunia yang dipilih. Jangan menambahkan gurun hanya sebagai petak pasir tanpa sebab geografis/lore.

### D-03 — Aturan hitung luas
Pilih salah satu model:
- **Petak eksklusif:** setiap titik lahan masuk tepat ke satu zona utama; jalan yang melintas zona lain dihitung sebagai atribut jaringan, bukan luas terpisah.
- **Kategori penggunaan:** satu lokasi boleh memiliki beberapa fungsi, tetapi angka kategori tidak boleh dijumlahkan sebagai total luas unik.

Masterplan sekarang menggunakan prinsip petak eksklusif, tetapi uraian Z06/Z13 masih memerlukan definisi geometri yang tegas.

### D-04 — Makna “distrik” dan “zona”
Tetapkan istilah:
- **Zona** = alokasi ruang utama untuk perhitungan luas.
- **Distrik** = wilayah pengalaman/administratif yang dapat terdiri dari satu atau beberapa zona.
- **Bioma** = klasifikasi lingkungan yang dapat melintasi zona dan distrik.

Definisi ini adalah usulan untuk menghindari pencampuran istilah; perlu diterima atau direvisi sebelum spesifikasi data dibakukan.

### D-05 — Kebebasan pemain yang bisa dibuktikan
Tetapkan sejumlah skenario contoh, misalnya pemain berfokus pada perdagangan, eksplorasi alam, kehidupan komunitas, atau pengembangan tempat tinggal. Setiap skenario harus menyebut:
1. apa yang bisa dilakukan pemain;
2. apa yang tidak diwajibkan;
3. pilihan rute atau pendekatan;
4. konsekuensi yang dapat diamati;
5. bagaimana dunia merespons.

Jangan menyebut sistem ini sudah ada; ini adalah target desain untuk ditulis dan ditinjau.

## 5. Penilaian zona masterplan saat ini

Pembagian 13 zona berjumlah tepat 10.000 ha secara aritmetika. Itu hanya membuktikan penjumlahan tabel, bukan membuktikan geometri, jarak, atau kelayakan desainnya.

| Zona | Luas | Penilaian awal | Yang masih dibutuhkan |
|---|---:|---|---|
| Z01 Pusat masjid/peradaban | 500 ha | Skala sangat besar untuk pusat inti; mungkin mencakup plaza dan ruang publik luas. | Bedakan area inti masjid dari kawasan sipil pendukung dan ruang terbuka. |
| Z02 Pendidikan/riset | 500 ha | Bisa menjadi distrik pendidikan besar. | Tingkat institusi, kapasitas, relasi dengan permukiman, dan aktivitas pemain. |
| Z03 Permukiman/kampung | 900 ha | Berpotensi mendukung beberapa komunitas, tetapi belum ada pembagian kampung. | Jumlah permukiman konseptual, karakter, kepadatan, akses, dan rutinitas. |
| Z04 Perdagangan/ekonomi | 300 ha | Fungsi masuk akal, batas terhadap pasar lokal belum jelas. | Bedakan pasar utama, toko lingkungan, bengkel, gudang, dan jaringan dagang. |
| Z05 Kesehatan/pelayanan | 200 ha | Alokasi fungsi umum. | Hierarki layanan dan jangkauan tiap fasilitas. |
| Z06 Jalan/utilitas | 500 ha | Berpotensi tumpang tindih dengan zona lain. | Definisi petak eksklusif, bukan hanya jaringan yang melintasi peta. |
| Z07 Pertanian/pangan | 1.000 ha | Memerlukan air, tanah yang cocok, dan hubungan logistik. | Posisi relatif ke sungai, permukiman, pasar, serta habitat. |
| Z08 Hutan tropis/lahan basah | 1.800 ha | Area besar yang dapat menjadi tulang punggung eksplorasi/ekologi. | Pisahkan hutan, rawa, sungai, dan zona konservasi; bukan satu hamparan seragam. |
| Z09 Pegunungan/dataran tinggi | 1.700 ha | Skala besar dengan dampak besar pada konektivitas. | Elevasi, lereng, lembah, jalur yang bisa dilewati, dan sumber air. |
| Z10 Pesisir/pantai | 900 ha | Luas perlu dibaca sebagai wilayah pesisir, bukan garis pantai semata. | Garis pantai, tebing, pantai terbuka, desa nelayan, dan area konservasi. |
| Z11 Gurun/wilayah kering | 700 ha | Perlu alasan geografis yang jelas dalam dunia kompak. | Cekungan/efek bayangan hujan/lore, sumber air terbatas, dan batas transisi. |
| Z12 Budaya/rekreasi/eksplorasi | 500 ha | Berisiko menjadi kategori campuran yang menelan fungsi lain. | Definisikan apakah ini zona eksklusif atau fungsi yang hadir di seluruh dunia. |
| Z13 Koridor/utilitas regional | 500 ha | Fungsi logistik penting tetapi batas dengan Z06 kabur. | Bedakan simpul/depo/terminal dari jaringan jalan dan utilitas. |

## 6. Risiko pengalaman pemain

1. **Peta luas tetapi kosong:** hektare besar tidak menjamin aktivitas. Setiap wilayah perlu ritme penemuan, aktivitas berulang, dan alasan untuk kembali.
2. **Perjalanan melelahkan:** jarak harus diuji terhadap kecepatan gerak, landmark, tempat singgah, dan pilihan transportasi. Jangan menentukan menit perjalanan sebelum skala pengalaman diputuskan.
3. **Bioma terasa seperti level terpisah:** gunakan transisi, sungai, jalur, perubahan vegetasi, dan alasan iklim yang saling terhubung.
4. **Pusat masjid menjadi satu-satunya konten:** pusat harus penting, tetapi komunitas, alam, perdagangan, dan eksplorasi juga harus punya nilai intrinsik.
5. **Simulasi terlalu ambisius:** rutinitas NPC, ekonomi, ekologi, cuaca, dan transportasi semuanya dapat menjadi sistem besar. Tentukan pengalaman inti sebelum menjanjikan simulasi mendalam.
6. **Lore tidak dapat dilacak:** legenda tanpa sumber yang jelas dapat merusak kepercayaan pada dokumentasi.
7. **Dokumentasi menyamar sebagai produk:** spesifikasi, gambar, dan tabel bukan bukti ada build yang dapat dimainkan.

## 7. Urutan kerja dokumentasi yang direkomendasikan

### Tahap 1 — Bereskan keputusan dasar
1. Putuskan hubungan skala 1.000 ha dan 10.000 ha.
2. Tetapkan definisi zona, distrik, dan bioma.
3. Tetapkan aturan perhitungan luas yang tidak tumpang tindih.
4. Buat konsep geografi menyeluruh untuk empat bioma.

### Tahap 2 — Revisi masterplan induk
5. Perbarui masterplan agar keputusan dasar terlihat di bagian awal.
6. Tulis hubungan antarwilayah dan transisi bioma, bukan hanya deskripsi tiap zona.
7. Tinjau ulang Z01, Z06, Z12, dan Z13 karena fungsi/batasnya masih paling ambigu.
8. Tandai semua koordinat dan proporsi yang masih placeholder.

### Tahap 3 — Bangun dokumen turunan yang substantif
9. Buat dokumen geografi dunia dan hidrologi konseptual.
10. Buat satu dokumen mendalam untuk masing-masing dari empat bioma.
11. Buat rancangan kawasan masjid yang membedakan inti, pendukung, dan hubungan dengan kota/kampung.
12. Buat indeks 25 legenda dengan status sumber, tanpa mengisi fakta yang belum terverifikasi.
13. Tulis skenario gameplay untuk menguji kebebasan pemain.

### Tahap 4 — Persiapan tinjauan
14. Buat matriks keputusan dan daftar pertanyaan terbuka.
15. Setelah dokumen inti konsisten, susun paket ringkas untuk studio.
16. Jangan menyebut dokumen siap produksi sebelum ada tinjauan dan keputusan yang sesuai.

## 8. Kriteria penerimaan audit

- [x] Dokumen yang menjadi dasar audit disebutkan.
- [x] Perbedaan skala 1.000 ha dan 10.000 ha dicatat sebagai isu yang belum diputuskan.
- [x] Ketidakkonsistenan 12 distrik versus 13 zona dicatat.
- [x] Risiko tumpang tindih alokasi lahan dijelaskan.
- [x] Empat bioma diperlakukan sebagai satu persoalan geografi dunia, bukan empat deskripsi terpisah.
- [x] 25 legenda tidak diklaim terverifikasi.
- [x] Dokumen ini membedakan rancangan konseptual dari implementasi.
- [x] Ada urutan tindak lanjut yang tidak memerlukan coding atau pemilihan engine.

## 9. Status dan keputusan berikutnya

Audit ini adalah **DRAF untuk ditinjau**, bukan pengesahan masterplan. Keputusan yang paling berpengaruh adalah definisi hubungan kawasan 1.000 ha dengan dunia ECOS 10.000 ha. Setelah keputusan itu ditetapkan, masterplan induk dapat direvisi dan dokumen geografi dibuat dengan lebih sedikit asumsi yang saling bertentangan.

## 10. Riwayat perubahan

- 0.1 — Audit awal berdasarkan dokumen masterplan ECOS, spesifikasi konseptual, struktur dokumentasi, standar kualitas, dan README repo kawasan masjid.
