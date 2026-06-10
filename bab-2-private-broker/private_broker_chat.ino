#include <WiFi.h>
#include <PubSubClient.h>

// ==========================================
// ZONA KONFIGURASI 
// ==========================================
const char* ssid = "NAMA_WIFI";
const char* password = "PASSWORD_WIFI";

const char* mqtt_server = "xxx.emqxsl.com"; 
const int   mqtt_port = 15214;              
const char* mqtt_user = "user_esp32";       
const char* mqtt_pass = "pass_esp32";       

const char* topic_publish = "pelatihan/chat/dari_esp32";
const char* topic_subscribe = "pelatihan/chat/dari_pc";
// ==========================================

WiFiClient espClient;
PubSubClient client(espClient);

// Fungsi ini dipanggil otomatis setiap kali ada pesan MQTT masuk
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("\n[PESAN MASUK] dari MQTTX: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup_wifi() {
  Serial.print("Konek ke WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println(" Berhasil!");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Konek ke MQTT...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println(" Berhasil!");
      // Jangan lupa Subscribe setelah berhasil konek!
      client.subscribe(topic_subscribe); 
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); // Mengaktifkan fungsi penerima pesan
}

void loop() {
  if (!client.connected()) { reconnect(); }
  client.loop();

  // Membaca ketikan dari Serial Monitor Arduino IDE
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim(); // Membersihkan spasi/enter berlebih
    
    if (msg.length() > 0) {
      Serial.print("[KIRIM PESAN] ke MQTTX: ");
      Serial.println(msg);
      // Kirim pesan ke MQTT
      client.publish(topic_publish, msg.c_str());
    }
  }
}
