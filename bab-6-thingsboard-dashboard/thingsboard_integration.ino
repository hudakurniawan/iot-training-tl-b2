#include <WiFi.h>
#include <PubSubClient.h>
#include <PZEM004Tv30.h>

// ==========================================
// ZONA KONFIGURASI 
// ==========================================
const char* ssid = "NAMA_WIFI";
const char* password = "PASSWORD_WIFI";

// Konfigurasi Server ThingsBoard (Ubah jika menggunakan server private/berbeda)
const char* mqtt_server = "demo.thingsboard.io"; 
const int   mqtt_port = 8883;              

// Gunakan Access Token dari Device ThingsBoard Anda sebagai Username
const char* mqtt_user = "ACCESS_TOKEN_ANDA";       
const char* mqtt_pass = ""; // Dikosongkan untuk ThingsBoard

// Topik Standar ThingsBoard (JANGAN DIUBAH)
const char* topic_telemetry = "v1/devices/me/telemetry"; 
const char* topic_rpc_sub   = "v1/devices/me/rpc/request/+";

const int ledPin = 2; // Pin Modul Relay

// ==========================================
// STATE & VARIABEL GLOBAL
// ==========================================
#include <WiFiClientSecure.h>
WiFiClientSecure espClient;
PubSubClient client(espClient);
PZEM004Tv30 pzem(Serial2, 16, 17);

unsigned long lastMsg = 0; 
float batas_daya_maks = 500.0; 
bool is_tripped = false;       
bool is_relay_on = false;

// --- FUNGSI PENERIMA PERINTAH (RPC) DARI THINGSBOARD ---
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) { message += (char)payload[i]; }
  
  Serial.printf("\n[RPC MASUK] Pesan: %s\n", message.c_str());

  // ThingsBoard mengirim perintah dalam format JSON. Contoh: {"method":"setRelay","params":true}
  // Catatan: Untuk keamanan produksi, disarankan menggunakan library ArduinoJson. 
  // Di sini kita menggunakan pencarian teks (indexOf) untuk penyederhanaan pembelajaran.

  // 1. KONTROL RELAY (ON/OFF)
  if (message.indexOf("setRelay") > 0) {
    if (message.indexOf("true") > 0) {
      if (is_tripped) {
        Serial.println("-> GAGAL ON: Sistem sedang TRIP! Lakukan RESET.");
      } else {
        digitalWrite(ledPin, HIGH);
        is_relay_on = true;
        Serial.println("-> Relay MENYALA");
      }
    } else if (message.indexOf("false") > 0) {
      digitalWrite(ledPin, LOW);
      is_relay_on = false;
      Serial.println("-> Relay MATI");
    }
  }
  
  // 2. MENGATUR BATAS DAYA
  else if (message.indexOf("setBatasDaya") > 0) {
    // Mengekstrak angka di sebelah "params":
    int indexValue = message.indexOf("\"params\":") + 9;
    int indexEnd = message.indexOf("}");
    if (indexValue > 9 && indexEnd > indexValue) {
      String valStr = message.substring(indexValue, indexEnd);
      batas_daya_maks = valStr.toFloat();
      Serial.printf("-> Batas Daya diubah menjadi %.1f Watt\n", batas_daya_maks);
    }
  }

  // 3. MERESET SISTEM
  else if (message.indexOf("resetSistem") > 0) {
    is_tripped = false;
    Serial.println("-> Sistem di-RESET. Relay siap digunakan.");
  }
}

void setup_wifi() {
  Serial.print("Konek ke WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println(" Berhasil!");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Konek ke ThingsBoard...");
    // ThingsBoard mengidentifikasi device murni dari Access Token (mqtt_user)
    if (client.connect("ESP32_Client", mqtt_user, mqtt_pass)) {
      Serial.println(" Berhasil!");
      client.subscribe(topic_rpc_sub); // Mendengarkan perintah dari Dashboard
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  setup_wifi();
  
  espClient.setInsecure(); // Bypass sertifikat SSL ThingsBoard
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) { reconnect(); }
  client.loop(); 

  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    float voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power();

    if (!isnan(voltage)) {
      // --- LOGIKA SMART TRIP ---
      if (power > batas_daya_maks && is_relay_on && !is_tripped) {
        digitalWrite(ledPin, LOW);
        is_relay_on = false;
        is_tripped = true;
        Serial.println("!!! PERINGATAN: OVERLOAD !!! Relay diputus otomatis.");
      }

      // Publish telemetri ke ThingsBoard
      String payload = "{\"tegangan\": " + String(voltage, 1) + 
                       ", \"arus\": " + String(current, 2) + 
                       ", \"daya\": " + String(power, 1) + 
                       ", \"batas_daya\": " + String(batas_daya_maks, 1) +
                       ", \"is_tripped\": " + String(is_tripped ? "true" : "false") + 
                       ", \"relay_status\": " + String(is_relay_on ? "true" : "false") + "}";
                       
      client.publish(topic_telemetry, payload.c_str());
    }
  }
}
