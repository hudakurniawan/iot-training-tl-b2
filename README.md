# Pelatihan IoT - Kurikulum 6 Bab

Selamat datang di program Pelatihan IoT. Repositori ini berisi kode dan sumber daya untuk setiap bab dari kurikulum, membawa Anda dari konektivitas cloud dasar hingga menjadi perangkat IoT yang siap produksi dengan sistem *smart trip* dan integrasi *dashboard*.

## Peta Jalan Kursus (Roadmap)

### [Bab 1: Halo MQTT](./bab-1-hello-mqtt/)
- Konektivitas Wi-Fi dasar.
- Menghubungkan ke Broker MQTT publik (EMQX).
- Konsep dasar Publish dan Subscribe.

### [Bab 2: Private Broker & Komunikasi Dasar](./bab-2-private-broker/)
- Membuat Private MQTT Broker menggunakan EMQX Serverless (gratis).
- Pengujian komunikasi antar PC menggunakan MQTTX.
- Mengirim pesan *chat* antara ESP32 dan PC menggunakan pemisahan topik.

### [Bab 3: Integrasi Relay & Umpan Balik Status](./bab-3-relay-integration/)
- Menghubungkan Modul Relay.
- Menerima perintah ON/OFF dari PC.
- Logika percabangan bersyarat: ESP32 hanya membalas pesan konfirmasi jika status relay benar-benar berubah.

### [Bab 4: Integrasi PZEM-004T & Topik Sensor](./bab-4-pzem-integration/)
- Menghubungkan sensor arus dan tegangan AC (PZEM-004T).
- Mengelompokkan data menggunakan format JSON manual.
- Menggunakan beberapa topik sekaligus: Topik Chat, Topik Sensor, dan Topik Kontrol.

### [Bab 5: Logika Pembatas Daya (Smart Trip)](./bab-5-smart-trip-logic/)
- ESP32 menerima pengaturan batas daya puncak dari aplikasi klien MQTT.
- Memori *state*: Jika pembacaan sensor PZEM melebihi batas daya, sistem otomatis memutuskan relay (Trip).
- Membutuhkan perintah 'RESET' eksplisit sebelum sistem dapat dihidupkan kembali.

### [Bab 6: Integrasi Dashboard ThingsBoard](./bab-6-thingsboard-dashboard/)
- Beralih dari penggunaan klien MQTTX ke antarmuka pengguna grafis profesional.
- Membuat grafik (*chart*) dari aliran data PZEM-004T.
- Mengendalikan relay dan batas daya melalui *widget* tombol interaktif di *Dashboard* web.
