#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <PZEM004Tv30.h> // Library PZEM-004t V3.0

// ==========================================
// ZONA KONFIGURASI 
// ==========================================
const char* ssid = "NAMA_WIFI";
const char* password = "PASSWORD_WIFI";

const char* mqtt_server = "xxx.emqxsl.com"; 
const int   mqtt_port = 15214;              
const char* mqtt_user = "user_esp32";       
const char* mqtt_pass = "pass_esp32";

// Pemisahan Topik agar rapi
const char* topic_publish_chat = "pelatihan/chat/dari_esp32";
const char* topic_publish_sensor = "pelatihan/sensor/pzem"; 
const char* topic_subscribe = "pelatihan/chat/dari_pc";

const int ledPin = 2; // Pin LED built-in atau Relay
// ==========================================

WiFiClientSecure espClient;
PubSubClient client(espClient);

// Inisialisasi PZEM-004t di Hardware Serial 2 (RX = Pin 16, TX = Pin 17)
PZEM004Tv30 pzem(Serial2, 16, 17);

unsigned long lastMsg = 0; // Variabel untuk timer pengiriman data

// --- FUNGSI PENERIMA PESAN (KONTROL LED) ---
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) { message += (char)payload[i]; }
  
  message.trim(); 
  message.toUpperCase(); 

  if (message == "ON") {
    digitalWrite(ledPin, HIGH);
    Serial.println("-> Perintah Diterima: LED MENYALA");
    client.publish(topic_publish_chat, "Status: LED MENYALA");
  } 
  else if (message == "OFF") {
    digitalWrite(ledPin, LOW);
    Serial.println("-> Perintah Diterima: LED MATI");
    client.publish(topic_publish_chat, "Status: LED MATI");
  }
}

void setup_wifi() {
  Serial.print("Konek ke WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println(" Berhasil!");
  espClient.setInsecure(); // Bypass sertifikat SSL
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Konek ke MQTT (SSL)...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println(" Berhasil!");
      client.subscribe(topic_subscribe); 
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
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) { reconnect(); }
  client.loop(); // Wajib agar ESP32 terus mendengarkan pesan masuk

  // --- LOGIKA PEMBACAAN SENSOR (Setiap 5 Detik) ---
  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    // Baca data dari PZEM-004t
    float voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power();

    // Cek apakah sensor berhasil dibaca
    if (isnan(voltage)) {
      Serial.println("Gagal membaca sensor PZEM. Cek kabel TX/RX!");
    } else {
      // Merakit data menjadi format JSON
      String payload = "{\"tegangan\": " + String(voltage, 1) + 
                       ", \"arus\": " + String(current, 2) + 
                       ", \"daya\": " + String(power, 1) + "}";
                       
      Serial.print("[KIRIM DATA PZEM] ");
      Serial.println(payload);
      
      // Kirim ke MQTT Cloud
      client.publish(topic_publish_sensor, payload.c_str());
    }
  }

  // --- LOGIKA CHAT VIA SERIAL MONITOR (Opsional) ---
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim(); 
    if (msg.length() > 0) {
      Serial.print("[KIRIM CHAT] ");
      Serial.println(msg);
      client.publish(topic_publish_chat, msg.c_str());
    }
  }
}
