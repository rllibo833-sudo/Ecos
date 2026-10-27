# ECOS — Spesifikasi Implementasi Masterplan

- Versi: 0.1
- Status: spesifikasi awal untuk prototipe; belum menjadi data geospasial final.
- Berkaitan dengan: `MASTERPLAN-ECOS-10000-HEKTARE.md`

## 1. Tujuan teknis

Mengubah masterplan konseptual menjadi peta yang dapat dibangun dan diuji dalam game. Spesifikasi ini memisahkan data dunia dari engine agar batas distrik, titik penting, jalan, dan bioma dapat direvisi tanpa menulis ulang seluruh sistem.

## 2. Sistem koordinat

Untuk prototipe, gunakan bidang lokal 10.000 m × 10.000 m sebagai batas persegi kerja.

- Origin: sudut barat daya pada (x=0, y=0).
- Sumbu X: ke timur, 0–10.000 m.
- Sumbu Y: ke utara, 0–10.000 m.
- Z: elevasi relatif dalam meter, ditentukan dari terrain.
- Konvensi: 1 unit data = 1 meter; engine boleh menggunakan skala berbeda jika ada konversi tunggal dan teruji.
- Batas persegi ini hanya wadah teknis. Bentuk playable world boleh tidak beraturan; area di luar batas daratan dapat berupa laut, tebing, pegunungan penghalang, atau batas dunia yang disengaja.

Koordinat ini bukan koordinat geografis dunia nyata dan tidak menunjukkan lokasi kawasan yang sebenarnya.

## 3. Struktur grid dan streaming

Gunakan grid konseptual 20 × 20 sel untuk pengelolaan wilayah. Jika wadah kerja 10 km × 10 km dibagi rata, setiap sel berukuran 500 m × 500 m = 25 ha. Total grid 400 sel = 10.000 ha.

- Grid digunakan untuk authoring, pengelolaan data, pengujian, dan streaming.
- Grid bukan batas wajib distrik; distrik dapat melintasi beberapa sel.
- Simpan ID sel, jenis bioma dominan, distrik, tingkat detail, dan kebutuhan aset.
- Sel yang jauh dari pemain dapat memakai simulasi ringkas atau tidak dimuat sebagai geometri penuh.
- Jangan membuat seluruh 100 km² penuh aset detail sekaligus.

## 4. Layer peta yang perlu dipisahkan

1. World boundary — batas playable world.
2. Elevation/terrain — ketinggian, kemiringan, lembah, punggung bukit.
3. Hydrology — sumber air, sungai, danau, rawa, drainase.
4. Biomes — tropis, pegunungan, pesisir, gurun, lahan pertanian, urban.
5. Districts — 12 distrik fungsional dari masterplan.
6. Land use — bangunan, hunian, fasilitas, pertanian, konservasi.
7. Roads and paths — jalan regional, lokal, servis, trek, jalur pesisir.
8. Points of interest (POI) — masjid, pasar, sekolah, klinik, oasis, air terjun, viewpoint.
9. Gameplay — misi, interaksi, sumber daya, spawn, aktivitas NPC.
10. Streaming/LOD — prioritas pemuatan dan tingkat detail aset.

Setiap layer harus punya ID stabil, versi, dan dokumentasi skema.

## 5. Model data awal

Contoh skema konseptual JSON untuk distrik:

```json
{
  "id": "D01",
  "name": "Jantung ECOS",
  "zone_id": "Z01",
  "area_ha_target": 500,
  "biome": "urban_garden",
  "role": ["spiritual_center", "navigation_landmark"],
  "poi_ids": [],
  "road_node_ids": [],
  "status": "planned"
}
```

Contoh titik penting:

```json
{
  "id": "POI_MOSQUE_MAIN",
  "name": "Masjid Utama ECOS",
  "type": "landmark",
  "district_id": "D01",
  "position_m": {"x": 5000, "y": 5000, "z": 0},
  "status": "placeholder"
}
```

Koordinat (5000, 5000) di atas hanyalah placeholder untuk menguji skema, bukan penempatan final. Jangan menganggap lokasi pusat atau elevasinya telah disetujui.

Contoh ruas jalan:

```json
{
  "id": "ROAD_REGIONAL_001",
  "class": "regional",
  "from_node": "NODE_001",
  "to_node": "NODE_002",
  "surface": "paved",
  "vehicle_access": true,
  "status": "planned"
}
```

Gunakan validasi untuk memastikan semua ID unik, semua referensi node/POI ada, dan semua nilai luas berjumlah tepat 10.000 ha.

## 6. Rancangan topologi awal

Sebelum menggambar geometri final, tentukan hubungan topologi berikut:
- Jantung ECOS terhubung ke Kota Ilmu, Kampung Kehidupan, Pasar dan Ekonomi, Pelayanan Warga.
- Sabuk Pangan terhubung ke pasar dan beberapa kampung melalui jalan lokal/servis.
- Hutan Tropis terhubung ke titik akses eksplorasi dan aliran sungai; jalan kendaraan dibatasi.
- Pegunungan terhubung ke hutan melalui lembah atau jalur kaki, bukan jaringan jalan padat.
- Pesisir terhubung ke permukiman/pasar melalui jalan regional dan dapat memiliki jalur laut di tahap lanjutan.
- Gurun memiliki akses regional ke pos persinggahan/oasis; jangan memaksa jalan melewati semua area sensitif.
- Koridor dan Utilitas Regional menghubungkan jaringan, depot, dan simpul logistik tanpa menjadi satu pusat tunggal.

Topologi ini tidak menentukan koordinat final. Penempatan aktual perlu mempertimbangkan elevasi, sungai, jarak perjalanan, dan ritme eksplorasi.

## 7. Perencanaan perjalanan

Buat matriks perjalanan untuk semua pasangan POI utama setelah koordinat dan jaringan tersedia. Ukur waktu di build menggunakan kecepatan karakter/kendaraan yang benar-benar diimplementasikan.

Kriteria uji:
- Setiap POI utama dapat dicapai melalui rute yang sah.
- Tidak ada jalan yang berakhir tanpa alasan desain atau titik balik.
- Jalur alam memiliki penanda navigasi yang cukup.
- Tidak ada kebutuhan melintasi terrain yang tidak bisa dilewati untuk mencapai fasilitas wajib.
- Waktu perjalanan panjang diimbangi oleh penemuan, aktivitas, tempat singgah, atau opsi transportasi.
- Jangan mengarang waktu tempuh sebelum simulasi tersedia.

## 8. Tahapan produksi

### Tahap A — Data masterplan
- Buat data zona dan distrik dengan ID stabil.
- Validasi total area 10.000 ha dan tidak ada overlap dalam perhitungan.
- Tetapkan sistem koordinat dan batas dunia.
- Buat daftar POI dan kelas jalan.

**Selesai bila:** validator lulus dan data dapat dibaca oleh tooling tanpa error.

### Tahap B — Blockout peta
- Terrain kasar, sungai utama, elevasi, garis pantai, wilayah kering.
- Tempatkan placeholder pusat masjid dan beberapa distrik.
- Buat satu rute dari pusat menuju satu bioma luar.

**Selesai bila:** pemain dapat bergerak melalui area blockout, skala terasa masuk akal, dan rute dapat diuji.

### Tahap C — Vertical slice
- Bangun satu area kecil yang punya landmark, bangunan yang dapat dimasuki atau digunakan, NPC, interaksi, satu aktivitas, dan save/load minimal.
- Gunakan aset sementara bila perlu, tetapi jangan mengklaim final art.
- Uji performa di perangkat target sedini mungkin.

**Selesai bila:** ada build yang dapat dimainkan dan langkah pengujian yang dapat diulang.

### Tahap D — Bioma pertama
- Pilih hutan tropis sebagai bioma eksplorasi pertama karena dapat menguji terrain, vegetasi, air, navigasi dan streaming.
- Implementasikan satu mekanik khas, misalnya menemukan air terjun atau jalur tersembunyi.
- Tambahkan optimasi dan pengujian perangkat.

**Selesai bila:** bioma bisa dijelajahi tanpa blocker kritis dan punya aktivitas nyata.

### Tahap E — Simulasi dan perluasan
- Rutinitas NPC, ekonomi sederhana, pertanian, transportasi, lalu bioma berikutnya.
- Integrasikan lore dan konsep repo setelah audit sumber.
- Tambah wilayah berdasarkan hasil playtest, bukan sekadar mengejar persentase peta.

## 9. Kriteria penerimaan (acceptance criteria)

- [ ] Total luas target 10.000 ha, dengan 13 zona dan ID unik.
- [ ] Sistem koordinat terdokumentasi dan konsisten.
- [ ] Grid 20 × 20 terdefinisi sebagai alat authoring, bukan batas distrik.
- [ ] Layer terrain, hidrologi, bioma, distrik, jalan, POI, gameplay dan streaming terpisah.
- [ ] Validasi menolak ID duplikat, referensi rusak, luas tidak cocok, dan koordinat di luar batas.
- [ ] Setidaknya satu rute dari pusat ke distrik luar dapat diuji dalam prototipe.
- [ ] Satu vertical slice dapat dimainkan; dokumen atau gambar saja tidak dihitung sebagai build.
- [ ] Performa diukur pada perangkat target sebelum memperluas dunia.
- [ ] Data legenda dan kontribusi repo ditandai belum diverifikasi sampai sumber asli diperiksa.

## 10. Risiko dan keputusan yang belum dikunci

- Bentuk batas dunia dan posisi absolut tiap distrik belum final.
- Alokasi luas adalah baseline konsep; perlu diperiksa ulang saat geometri nyata dibuat.
- Bioma gurun dalam wilayah 100 km² memerlukan penjelasan geografis/lore yang konsisten.
- Engine, platform target, kualitas grafis, skala NPC dan model streaming belum diputuskan.
- Nama serta kisah 25 legenda belum dikonfirmasi dari data sumber.
- Peran LynOS, PusingCuk, Mnemos-RLI, Steadfast dan repo lainnya belum boleh dianggap sebagai fitur final tanpa audit.

## 11. Non-goals

Dokumen ini tidak menyatakan bahwa peta sudah dibangun, game sudah playable, repo lain sudah terintegrasi, atau data lahan sudah disurvei. Semua itu memerlukan implementasi dan bukti yang terpisah.
