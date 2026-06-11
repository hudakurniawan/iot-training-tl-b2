#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <PZEM004Tv30.h>

// ==========================================
// ZONA KONFIGURASI 
// ==========================================
const char* ssid = "NAMA_WIFI";
const char* password = "PASSWORD_WIFI";

const char* mqtt_server = "xxx.emqxsl.com"; 
const int   mqtt_port = 15214;              
const char* mqtt_user = "user_esp32";       
const char* mqtt_pass = "pass_esp32";

// Topik Publish
const char* topic_pub_sensor = "pelatihan/sensor/pzem"; 
const char* topic_pub_alarm  = "pelatihan/notifikasi/alarm";

// Topik Subscribe
const char* topic_sub_kontrol = "pelatihan/chat/dari_pc";
const char* topic_sub_batas   = "pelatihan/pengaturan/batas_daya";
const char* topic_sub_reset   = "pelatihan/perintah/reset";

const int ledPin = 2; // Pin Modul Relay

// ==========================================
// STATE & VARIABEL GLOBAL
// ==========================================
WiFiClientSecure espClient;
PubSubClient client(espClient);
PZEM004Tv30 pzem(Serial2, 16, 17);

unsigned long lastMsg = 0; 

float batas_daya_maks = 200.0; // Default batas daya 200 Watt
bool is_tripped = false;       // Memori status trip (pemutus arus)
bool is_relay_on = false;

// --- FUNGSI PENERIMA PESAN ---
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) { message += (char)payload[i]; }
  message.trim(); 
  
  Serial.printf("\n[MQTT MASUK] Topik: %s | Pesan: %s\n", topic, message.c_str());

  // 1. MENGATUR BATAS DAYA
  if (String(topic) == topic_sub_batas) {
    batas_daya_maks = message.toFloat();
    Serial.printf("-> Pengaturan Baru: Batas Daya diubah menjadi %.1f Watt\n", batas_daya_maks);
  }
  
  // 2. MERESET SISTEM (RESET TRIP)
  else if (String(topic) == topic_sub_reset) {
    if (message.equalsIgnoreCase("RESET")) {
      is_tripped = false;
      Serial.println("-> Sistem di-RESET. Relay sekarang bisa dinyalakan kembali.");
      client.publish(topic_pub_alarm, "INFO: Sistem telah di-RESET.");
    }
  }

  // 3. KONTROL RELAY MANUAL (ON/OFF)
  else if (String(topic) == topic_sub_kontrol) {
    message.toUpperCase();
    
    if (message == "ON") {
      if (is_tripped) {
        Serial.println("-> GAGAL: Sistem sedang TRIP! Lakukan RESET terlebih dahulu.");
        client.publish(topic_pub_alarm, "GAGAL ON: Sistem masih dalam kondisi TRIP.");
      } else {
        digitalWrite(ledPin, HIGH);
        is_relay_on = true;
        Serial.println("-> Relay MENYALA");
      }
    } 
    else if (message == "OFF") {
      digitalWrite(ledPin, LOW);
      is_relay_on = false;
      Serial.println("-> Relay MATI");
    }
  }
}

void setup_wifi() {
  Serial.print("Konek ke WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println(" Berhasil!");
  espClient.setInsecure(); 
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Konek ke MQTT...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println(" Berhasil!");
      client.subscribe(topic_sub_kontrol); 
      client.subscribe(topic_sub_batas);
      client.subscribe(topic_sub_reset);
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Pastikan relay mati saat mulai

  setup_wifi();
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
      // --- LOGIKA SMART TRIP / PEMUTUS ARUS ---
      // Jika daya melebihi batas, DAN relay sedang menyala, DAN belum trip
      if (power > batas_daya_maks && is_relay_on && !is_tripped) {
        // Matikan relay seketika!
        digitalWrite(ledPin, LOW);
        is_relay_on = false;
        is_tripped = true; // Kunci sistem
        
        Serial.println("!!! PERINGATAN: OVERLOAD !!! Relay diputus otomatis.");
        client.publish(topic_pub_alarm, "BAHAYA: OVERLOAD! Beban dimatikan otomatis.");
      }

      // Publish telemetri
      String payload = "{\"tegangan\": " + String(voltage, 1) + 
                       ", \"arus\": " + String(current, 2) + 
                       ", \"daya\": " + String(power, 1) + 
                       ", \"batas_daya\": " + String(batas_daya_maks, 1) +
                       ", \"status_trip\": " + String(is_tripped ? "true" : "false") + "}";
                       
      client.publish(topic_pub_sensor, payload.c_str());
    }
  }
}
