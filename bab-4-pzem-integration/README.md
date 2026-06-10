# Bab 4: Integrasi PZEM-004T & Topik Sensor

Di bab ini, kita menambahkan kemampuan membaca sensor arus AC nyata menggunakan PZEM-004T. Kita juga memperkenalkan perakitan format **JSON manual** (`{"tegangan": 220, "arus": 1.2}`) sehingga sistem cloud dapat dengan mudah mem-parsing variabel-variabel tersebut.

### Diagram Alir (Flowchart) Komunikasi
```mermaid
graph TD
    subgraph "ESP32 (Hardware)"
        SENS[Sensor PZEM-004T] -->|1. Baca Data Kelistrikan| ESP[Mikrokontroler ESP32]
        ESP -->|4. Aktifkan Relay| REL[Modul Relay]
    end

    subgraph "Jaringan Internet"
        BROKER((Private Broker EMQX))
    end

    subgraph "Klien (Laptop PC)"
        MQTTX[Aplikasi MQTTX]
    end

    %% Aliran Data Telemetri (Naik ke Cloud)
    ESP -->|2. Publish JSON ke topik sensor| BROKER
    BROKER -->|Meneruskan Data| MQTTX

    %% Aliran Perintah Kontrol (Turun ke Perangkat)
    MQTTX -->|3. Publish perintah ke topik kontrol| BROKER
    BROKER -->|Meneruskan Perintah| ESP
```

### Diagram Pengabelan (Wiring)

```mermaid
graph LR
    subgraph ESP32
        GND[GND]
        V5[VIN / 5V]
        TX2[TX2 / Pin 17]
        RX2[RX2 / Pin 16]
        G2[GPIO 2]
    end

    subgraph PZEM-004T
        PGND[GND]
        P5V[5V]
        PRX[RX]
        PTX[TX]
    end
    
    subgraph Relay Module
        RGND[GND]
        RVCC[VCC]
        RIN[IN]
    end

    GND --- PGND
    V5 --- P5V
    TX2 -.-> PRX
    RX2 <-.- PTX
    
    GND --- RGND
    V5 --- RVCC
    G2 -.Sinyal.-> RIN
```
