# Bab 2: Private Broker & Komunikasi Dasar

Bab ini membahas cara menggunakan EMQX Serverless untuk membuat Private MQTT Broker. Kita akan memisahkan lalu lintas pesan ke dalam dua topik berbeda agar tidak saling bertabrakan:
- ESP32 mem-publish ke `pelatihan/chat/dari_esp32`
- PC (MQTTX) mem-publish ke `pelatihan/chat/dari_pc`

### Diagram Alir (Flowchart) Komunikasi
```mermaid
graph TD
    ESP[ESP32] -- Publish ke topic:\npelatihan/chat/dari_esp32 --> BROKER((Private Broker\nEMQX Serverless))
    BROKER -- Forward Pesan --> PC[Aplikasi MQTTX di PC]
    
    PC -- Publish ke topic:\npelatihan/chat/dari_pc --> BROKER
    BROKER -- Forward Pesan --> ESP
```

### Diagram Pengabelan (Wiring)
*(Pada tahap ini belum ada pengabelan khusus karena kita baru mensimulasikan interaksi chat (teks) antara Serial Monitor dan MQTTX)*

```mermaid
graph LR
    ESP[ESP32 Board]
    PC[Laptop / PC]
    USB((Kabel USB))
    WIFI((Koneksi Wi-Fi))
    
    ESP <-->|Serial Monitor Chat| USB <--> PC
    ESP -.->|Koneksi MQTT via Internet| WIFI
```
