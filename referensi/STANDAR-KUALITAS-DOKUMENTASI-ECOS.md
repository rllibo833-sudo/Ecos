# Standar Kualitas Dokumentasi ECOS

**Status:** RANCANGAN  
**Tujuan:** mencegah dokumentasi ECOS menjadi kumpulan file kosong, slogan, atau klaim fitur tanpa rancangan yang dapat ditinjau.  
**Batas:** standar dokumentasi desain; tidak memilih engine dan tidak mengimplementasikan game.

Dokumen ini melengkapi [Rancangan Struktur Dokumentasi ECOS v1.0](RANCANGAN-STRUKTUR-DOKUMENTASI-ECOS-V1.md). Struktur folder menentukan *di mana* topik dibahas; standar ini menentukan *seberapa matang* isinya harus dibuat.

## 1. Bar kualitas minimum

Sebuah dokumen tidak dianggap matang hanya karena memiliki judul, daftar bullet, atau banyak halaman. Dokumen harus menjawab pertanyaan desain yang relevan dan dapat diperiksa.

Setiap dokumen yang substantif harus memuat:

1. **Tujuan:** keputusan atau pemahaman apa yang harus dihasilkan.
2. **Ruang lingkup:** hal yang termasuk dan tidak termasuk.
3. **Pernyataan desain:** rancangan yang jelas, bukan hanya aspirasi.
4. **Contoh konkret:** setidaknya satu contoh yang memperlihatkan rancangan bekerja.
5. **Hubungan:** dokumen dan sistem lain yang terdampak.
6. **Aturan dan batasan:** apa yang boleh, tidak boleh, atau masih terbuka.
7. **Kasus tepi/risiko:** keadaan yang dapat membuat rancangan gagal atau tidak konsisten.
8. **Sumber dan asumsi:** bukti, ide asli, adaptasi, dan hal yang belum diverifikasi dipisahkan.
9. **Kriteria penerimaan:** cara pembaca menentukan apakah dokumen memenuhi tujuannya.
10. **Pertanyaan terbuka:** keputusan yang belum dibuat dan siapa yang perlu meninjaunya.

Dokumen referensi visual dapat menggunakan contoh gambar atau tautan sebagai pendukung, tetapi tetap harus menjelaskan aspek yang dirujuk dan alasan relevansinya. Jangan memakai gambar sebagai pengganti spesifikasi.

## 2. Template wajib dokumen desain

Gunakan template ini sebagai kerangka umum. Bagian yang tidak relevan boleh ditandai “Tidak berlaku” beserta alasannya; jangan dihapus diam-diam jika dibutuhkan oleh pembaca.

```markdown
# [Judul dokumen]

- ID: ECOS-[AREA]-[NOMOR]
- Status: RANCANGAN | PERLU-VERIFIKASI | DRAF | MENUNGGU-PERSETUJUAN | DISETUJUI
- Versi: 0.1
- Pemilik keputusan: Pemilik proyek ECOS
- Terakhir ditinjau: Belum ditinjau
- Dokumen terkait: [tautan relatif]

## 1. Tujuan
Apa yang perlu dipahami atau diputuskan setelah membaca dokumen ini?

## 2. Ruang lingkup
### Termasuk
### Tidak termasuk

## 3. Ringkasan desain
Jelaskan rancangan dengan bahasa langsung dan spesifik.

## 4. Rincian desain
Gunakan subbagian, tabel, daftar aturan, atau diagram yang sesuai dengan topik.

## 5. Contoh konkret
Tunjukkan satu skenario, wilayah, aktivitas, atau keputusan yang menerapkan rancangan.

## 6. Hubungan dengan bagian ECOS lainnya
Apa yang menjadi masukan? Apa yang bergantung pada dokumen ini? Apa dampaknya bila rancangan berubah?

## 7. Batasan, risiko, dan kasus tepi
Catat konflik, pengecualian, dan keadaan yang belum terselesaikan.

## 8. Sumber, asumsi, dan keputusan
Pisahkan sumber yang diperiksa, gagasan baru ECOS, asumsi, dan keputusan yang telah disetujui.

## 9. Kriteria penerimaan
Daftar hal yang harus benar agar dokumen dapat disetujui.

## 10. Pertanyaan terbuka
Catat hal yang belum diputuskan tanpa menyamarkannya sebagai fakta.

## 11. Riwayat perubahan
- 0.1 — Draf awal; belum disetujui.
```

## 3. Tambahan wajib berdasarkan jenis dokumen

### A. Geografi dan masterplan
Dokumen spasial perlu menjelaskan:
- batas wilayah dan satuan yang digunakan;
- hubungan dan transisi ke wilayah tetangga;
- akses masuk/keluar dan rute alternatif;
- alasan penempatan wilayah;
- fungsi, landmark, dan aktivitas yang didukung;
- ketergantungan pada air, elevasi, iklim, atau sumber daya;
- apakah luas merupakan angka konseptual, hasil hitung, atau data terverifikasi;
- cara menghindari penghitungan ganda antar zona.

**Kriteria:** pembaca dapat menjelaskan lokasi, fungsi, hubungan, dan alasan desain wilayah tanpa harus menebak dari nama zona.

### B. Bioma dan ekologi
Setiap bioma harus memiliki:
- identitas visual dan karakter lingkungan;
- kondisi geografis/iklim yang menjadi asumsi desain;
- flora, fauna, dan habitat yang relevan;
- sumber daya dan batas ketersediaannya;
- risiko atau tantangan perjalanan;
- aktivitas pemain yang masuk akal;
- transisi menuju bioma tetangga;
- konsekuensi untuk permukiman, ekonomi, dan konservasi.

**Kriteria:** empat bioma terasa berbeda dalam fungsi dan pengalaman, bukan hanya berbeda warna atau tekstur.

### C. Kawasan masjid
Dokumen harus membedakan:
- kawasan inti dan zona pendukung;
- fungsi ibadah, pendidikan, pelayanan, budaya, ruang publik, dan ekonomi;
- akses pejalan kaki dan koneksi ke permukiman;
- kegiatan sehari-hari dan aktivitas khusus;
- peran kawasan terhadap dunia tanpa menjadikannya satu-satunya lokasi menarik;
- luas yang diasumsikan dan keputusan yang belum ditetapkan.

**Kriteria:** kawasan memiliki fungsi yang dapat dijelaskan melalui ruang, komunitas, dan aktivitas pemain; bukan hanya landmark besar.

### D. 25 legenda
Setiap entri legenda perlu memiliki kolom:
- ID tetap (LEG-01 sampai LEG-25);
- nama (atau “belum diverifikasi”);
- ringkasan yang diketahui;
- sumber dan bagian sumber;
- status verifikasi;
- hubungan lokasi/bioma;
- karakter atau komunitas terkait;
- potensi pengalaman pemain;
- hal yang dilarang untuk diasumsikan sebelum verifikasi.

**Kriteria:** jumlah entri tepat 25; setiap entri dapat dilacak; informasi kosong tetap dinyatakan kosong dan tidak diisi dengan karangan yang diklaim sebagai materi asli.

### E. Gameplay dan kebebasan pemain
Dokumen gameplay perlu menjelaskan:
- tindakan yang dapat dilakukan pemain;
- apakah aktivitas wajib, opsional, atau bersyarat;
- pilihan yang tersedia;
- konsekuensi dan umpan balik;
- kebutuhan sumber daya/waktu bila relevan;
- dampak pada dunia, komunitas, atau progresi;
- jalur alternatif untuk gaya bermain berbeda;
- batasan dan potensi eksploitasi desain.

**Kriteria:** setidaknya ada skenario konkret yang menunjukkan dua pemain dapat memilih pendekatan berbeda dan memahami konsekuensinya.

### F. Karakter dan komunitas
Dokumen perlu menjelaskan:
- peran sosial dan aktivitas sehari-hari;
- kebutuhan, tujuan, rutinitas, dan hubungan;
- respons terhadap tindakan pemain;
- kaitan dengan lokasi dan ekonomi;
- apa yang diketahui, diasumsikan, atau belum dirancang.

**Kriteria:** NPC dan komunitas tidak hanya menjadi dekorasi atau pemberi misi; fungsi mereka dalam dunia dapat diterangkan.

### G. Paket studio
Paket studio harus:
- merangkum keputusan yang telah ditinjau;
- memberi tautan ke sumber detail;
- membedakan keputusan yang disetujui dari ide yang masih terbuka;
- menyebutkan ruang lingkup dan risiko secara jujur;
- menghindari klaim tentang build, fitur, ukuran produksi, atau kelayakan teknis yang belum diuji.

**Kriteria:** pembaca eksternal dapat memahami konsep dan batas ketidakpastiannya tanpa disesatkan.

## 4. Aturan status dan sumber

Gunakan status dokumen berikut secara konsisten:
- `RANCANGAN`: struktur atau tujuan diusulkan.
- `PERLU-VERIFIKASI`: klaim/sumber penting belum diperiksa.
- `DRAF`: isi tersedia, belum diajukan untuk persetujuan.
- `MENUNGGU-PERSETUJUAN`: siap ditinjau pemilik proyek.
- `DISETUJUI`: keputusan desain telah disetujui.
- `SIAP-UNTUK-PRODUKSI`: rincian desain cukup jelas untuk diserahkan ke proses produksi; bukan bukti implementasi.
- `DIGANTIKAN`: tidak lagi menjadi acuan aktif.

Untuk setiap klaim yang berasal dari repo atau sumber luar, catat lokasi sumber yang cukup spesifik untuk ditemukan kembali. Jika sumber tidak dapat diakses atau belum dibaca, tandai belum diverifikasi. Jangan menyimpulkan isi sumber dari nama repo atau ringkasan yang tidak diperiksa.

## 5. Pagar anti-klaim palsu

Jangan menyatakan hal berikut kecuali ada bukti yang sesuai:
- “sistem sudah berjalan” jika baru dijelaskan dalam dokumen;
- “dunia dapat dimainkan” jika belum ada build yang dapat diperiksa;
- “25 legenda sudah terverifikasi” jika sumbernya belum diperiksa satu per satu;
- “masterplan akurat” jika luas dan geometri masih konseptual;
- “konsep dari repo X sudah diintegrasikan” jika pemetaan dan persetujuannya belum dilakukan;
- “siap produksi” jika dokumen belum memenuhi kriteria desain dan belum ditinjau.

Gunakan frasa yang akurat seperti “diusulkan”, “konseptual”, “belum diverifikasi”, dan “menunggu persetujuan”.

## 6. Urutan kerja dokumentasi yang disarankan

1. Inventarisasi file yang sudah ada dan sumber konsep yang tersedia.
2. Tandai isi yang dapat dipertahankan, perlu verifikasi, bertentangan, atau belum tersedia.
3. Tetapkan glossary dan ID dokumen.
4. Matangkan geografi dan batas asumsi dunia.
5. Matangkan empat bioma dan transisinya.
6. Periksa aritmetika dan logika spasial masterplan.
7. Matangkan kawasan masjid serta hubungannya dengan permukiman.
8. Petakan 25 legenda tanpa mengarang materi yang belum diverifikasi.
9. Hubungkan lokasi dengan aktivitas dan pilihan pemain.
10. Ajukan setiap kelompok untuk ditinjau sebelum diberi status disetujui.

## 7. Kriteria penerimaan standar ini

- [x] Template mewajibkan rincian desain, contoh, hubungan, batasan, sumber, dan kriteria penerimaan.
- [x] Geografi, bioma, kawasan masjid, legenda, gameplay, karakter, dan paket studio memiliki persyaratan khusus.
- [x] Status dokumen membedakan rancangan dari keputusan dan implementasi.
- [x] Sumber yang belum diperiksa tidak boleh diklaim telah diverifikasi.
- [x] Tidak ada kewajiban coding atau memilih engine dalam tahap dokumentasi.
