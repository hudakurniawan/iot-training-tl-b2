# Bab 6: Integrasi Dashboard ThingsBoard

Pada bab ini, kita akan menghubungkan ESP32 secara **langsung** ke MQTT Broker bawaan ThingsBoard. Ini akan menyederhanakan arsitektur kita sekaligus memungkinkan kita menggunakan fitur *Dashboard* interaktif untuk memonitor data dan mengontrol alat.

## Penyesuaian Kode (`thingsboard_integration.ino`)
Perbedaan utama dari kode Bab 5 adalah:
1. **Server MQTT:** Menggunakan `demo.thingsboard.io` (atau server khusus Anda).
2. **Kredensial:** Hanya menggunakan **Access Token** yang dimasukkan ke variabel `mqtt_user`. `mqtt_pass` dibiarkan kosong.
3. **Topik Standar:** ThingsBoard mewajibkan penggunaan topik khusus:
   - Telemetri (Publish): `v1/devices/me/telemetry`
   - RPC / Perintah (Subscribe): `v1/devices/me/rpc/request/+`
4. **Format Perintah (RPC):** Tombol dari dasbor ThingsBoard mengirimkan data berbentuk JSON seperti `{"method": "setRelay", "params": true}`.

---

## Panduan Langkah Demi Langkah (Web ThingsBoard)

1. **Membuat Perangkat (Device):**
   - Masuk ke akun [ThingsBoard](https://demo.thingsboard.io/).
   - Buka menu **Entities** -> **Devices**.
   - Klik tombol **+ Add Device** -> **Add new device**.
   - Beri nama perangkat (misalnya: `Smart Breaker 01`) dan klik Add.

2. **Mendapatkan Access Token:**
   - Klik perangkat yang baru saja dibuat di daftar.
   - Klik tombol **Manage credentials**.
   - Salin **Access Token**. Masukkan token ini ke dalam baris `const char* mqtt_user = "..."` di kode Arduino Anda.

3. **Membuat Dashboard & Grafik Telemetri:**
   - Buka menu **Dashboards** dan klik **+ Add Dashboard**.
   - Buka dashboard baru tersebut, lalu klik tombol pensil (Edit mode) di pojok kanan bawah.
   - Klik **Add new widget**. Pilih **Charts** -> **Timeseries Line Chart**.
   - Pada bagian *Datasource*, tambahkan *Entity* perangkat Anda, lalu pilih *data key* `tegangan`, `arus`, dan `daya`.

4. **Membuat Tombol Kontrol (RPC):**
   > **PENTING:** Secara *default*, ThingsBoard akan mencoba meminta nilai awal (*initial value*) dari perangkat saat dasbor dimuat. Karena perangkat kita belum diprogram untuk merespons pertanyaan nilai awal, **pastikan Anda menghapus centang (uncheck) opsi "Retrieve value using method"** di bagian *Advanced* pada setiap widget di bawah ini untuk mencegah *error Request Timeout*.

   - **Tombol Sakelar (Relay):** Tambahkan *widget* **Control widgets** -> **Switch Control**.
     - Di tab *Advanced*, hapus centang *Retrieve value using method*.
     - Di bagian *Update value settings*, atur *RPC set value method* menjadi `setRelay`.
   - **Tombol Reset (Trigger):** Tambahkan *widget* **Control widgets** -> **RPC Button** (jangan gunakan *switch* untuk aksi reset).
     - Di tab *Advanced*, atur *RPC method* menjadi `resetSistem`.
     - Atur *RPC method params* menjadi `{}`.
     - Centang opsi **One-way request** (karena tombol ini tidak memerlukan balasan nilai).
   - **Input Batas Daya:** Tambahkan *widget* **Control widgets** -> **Knob Control** (atau Slider).
     - Pada UI ThingsBoard terbaru, masuk ke bagian **Behavior** di pengaturan widget.
     - Pada baris **Initial state**, klik ikon pensil (Edit) dan ubah agar **TIDAK** menggunakan *Execute RPC method* (Anda bisa mengubahnya ke *Static value* atau mematikannya). Ini mencegah *timeout* saat halaman baru dimuat.
     - Pada baris eksekusi perubahan nilai (misal: *Left/Right button click* atau *Value changed*), pastikan RPC method yang dipanggil adalah `setBatasDaya`.

---

### Diagram Alir (Flowchart) Komunikasi ThingsBoard
```mermaid
graph TD
    subgraph "Perangkat ESP32"
        ESP[Kirim Telemetri & Dengarkan RPC]
    end

    subgraph "ThingsBoard Cloud"
        TB_BROKER((MQTT Broker\nThingsBoard))
        RULE[Rule Engine]
        DB[(Database)]
        DASH[Web Dashboard]
    end

    %% Aliran Telemetri
    ESP -->|Publish JSON ke:\nv1/devices/me/telemetry| TB_BROKER
    TB_BROKER --> RULE
    RULE -->|Simpan Timeseries| DB
    DB -->|Render Grafik| DASH

    %% Aliran Perintah (RPC)
    DASH -->|Klik Tombol/Widget| RULE
    RULE -->|Kirim JSON RPC\nmethod & params| TB_BROKER
    TB_BROKER -->|Forward ke Topik:\nv1/devices/me/rpc/request/+| ESP
```

### Topologi Jaringan (Wiring Logis)
```mermaid
graph LR
    PZEM[PZEM-004T] -.Data Arus/Tegangan.-> ESP[ESP32]
    ESP -.Koneksi Wi-Fi.-> ROUTER[Router / Modem]
    ROUTER -.Internet.-> TB[ThingsBoard Cloud]
    
    USER[Pengguna / Laptop] -.Internet.-> TB
    TB -.Perintah Reset/Relay.-> ROUTER
    ROUTER -.-> ESP
    ESP -.-> RELAY[Modul Relay]
```
