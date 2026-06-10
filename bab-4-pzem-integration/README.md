# Bab 4: Integrasi PZEM-004T & Topik Sensor

Di bab ini, kita menambahkan kemampuan membaca sensor arus AC nyata menggunakan PZEM-004T. Kita juga memperkenalkan perakitan format **JSON manual** (`{"tegangan": 220, "arus": 1.2}`) sehingga sistem cloud dapat dengan mudah mem-parsing variabel-variabel tersebut.

### Diagram Alir (Flowchart) Komunikasi
```mermaid
graph TD
    ESP[ESP32] -- Baca Sensor tiap 5d --> SENS[PZEM-004T]
    ESP -- Rakit string JSON --> JSON{JSON Payload}
    JSON -- Publish --> BROKER((Private Broker))
    
    PC[MQTTX] -- Subscribe Sensor --> BROKER
    PC -- Publish Perintah Relay --> BROKER
    BROKER -- Meneruskan Perintah --> ESP
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
