# Bab 1: Halo MQTT

Bab ini mendemonstrasikan konektivitas Wi-Fi dasar dan Publish/Subscribe MQTT pada ESP32 menggunakan broker publik (tanpa autentikasi).

### Tautan Kode Sumber
Anda dapat menemukan dan mengunduh file kode lengkapnya (`kode_mqtt.ino`) di URL berikut:
[https://github.com/YakubFahimLuckyarno/Kode_MQTT/blob/main/kode_mqtt.ino](https://github.com/YakubFahimLuckyarno/Kode_MQTT/blob/main/kode_mqtt.ino)

---

### Diagram Alir (Flowchart) Komunikasi
```mermaid
graph TD
    A[ESP32] -- Publish 'Hi, I'm ESP32 ^^' --> B((Broker MQTT Publik\nbroker.emqx.io))
    B -- Subscribe --> A
    C[PC / MQTTX] -- Publish Pesan --> B
    B -- Forward Pesan --> C
```

### Diagram Pengabelan (Wiring)
*(Pada tahap ini belum ada pengabelan khusus karena kita hanya menggunakan modul internal ESP32)*

```mermaid
graph LR
    ESP[ESP32 Board]
    WIFI((Sinyal Wi-Fi))
    
    ESP -.->|Koneksi Nirkabel| WIFI
```
