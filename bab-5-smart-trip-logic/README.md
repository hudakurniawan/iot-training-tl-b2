# Bab 5: Logika Pembatas Daya (Smart Trip)

Pada bab ini, kita menambahkan "otak" pada ESP32. ESP32 tidak lagi hanya pasif menerima perintah, melainkan secara aktif memonitor data sensor. Jika daya (Watt) melebihi batas yang dikonfigurasi, ESP32 akan **memutus relay secara otomatis** (seperti *Miniature Circuit Breaker* / MCB pintar).

Sistem akan terkunci (Trip) dan mengabaikan perintah "ON" sampai pengguna secara eksplisit mengirimkan perintah "RESET".

### Diagram Alir (Flowchart) Logika Trip
```mermaid
graph TD
    BACA[ESP32 Membaca PZEM] --> CEK{Daya > Batas?}
    CEK -- Ya --> CEKON{Relay ON?}
    CEK -- Tidak --> AMAN[Sistem Aman]
    
    CEKON -- Ya --> TRIP[Matikan Relay!\nSet Status = TRIPPED]
    TRIP --> ALARM[Publish Alarm MQTT]
    
    CMD[Terima Perintah ON] --> IS_TRIP{Status TRIPPED?}
    IS_TRIP -- Ya --> TOLAK[Tolak Perintah]
    IS_TRIP -- Tidak --> NYALA[Nyalakan Relay]
    
    RST[Terima Perintah RESET] --> UNLOCK[Set Status = AMAN]
```

### Diagram Pengabelan (Wiring)
*(Sama seperti Bab 4, pengabelan fisik tidak berubah, hanya kecerdasan perangkat lunak yang bertambah).*

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
    G2 -.-> RIN
```
