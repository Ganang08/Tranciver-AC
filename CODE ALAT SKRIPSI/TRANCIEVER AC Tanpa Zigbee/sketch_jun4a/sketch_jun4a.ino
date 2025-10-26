#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <SoftwareSerial.h>

// ========== KONFIGURASI JARINGAN ==========
const char* ssid = "Yayaya";
const char* password = "082244026023";

// ========== KONFIGURASI FIREBASE ==========
#define FIREBASE_HOST "ac-app-b5549-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "wq9RwYVcTNOEN1qDuZWaYwlyvUQHl8pRvlj5jZKr"

// ========== DEKLARASI OBJEK ==========
FirebaseData fbData;
FirebaseAuth fbAuth;
FirebaseConfig fbConfig;
WiFiClient client;


// ========== STRUKTUR DATA AC ==========
struct ACState {
  bool lastState;
  int lastTemp;
  String path;
  String name;
  IPAddress serverIP;
  uint16_t serverPort;
};

// Inisialisasi array AC dengan IP dan port server masing-masing
ACState acs[] = {
  {false, 0, "/devices/AC_01", "Daikin 1pk", IPAddress(192,168,18,14), 5000}};
const int AC_COUNT = sizeof(acs) / sizeof(acs[0]);

void setup() {
  Serial.begin(115200);

  connectWiFi();

  fbConfig.host = FIREBASE_HOST;
  fbConfig.signer.tokens.legacy_token = FIREBASE_AUTH;

  Firebase.begin(&fbConfig, &fbAuth);
  Firebase.reconnectWiFi(true);

  Serial.println("\nMencoba koneksi ke Firebase...");
  unsigned long startTime = millis();
  while (!Firebase.ready() && millis() - startTime < 15000) {
    Serial.print(".");
    delay(500);
  }

  if (!Firebase.ready()) {
    Serial.println("\nGagal terkoneksi ke Firebase!");
    Serial.println("Penyebab: " + fbData.errorReason());
    while (1) delay(1000);
  }

  Serial.println("\nBerhasil terhubung ke Firebase!");
}

void loop() {
  if (!Firebase.ready() || WiFi.status() != WL_CONNECTED) {
    Serial.println("[ERROR] Koneksi terputus, mencoba reconnect...");
    connectWiFi();
    delay(5000);
    return;
  }

  for (int i = 0; i < AC_COUNT; i++) {
    checkACState(acs[i]);
    delay(200);
  }

  delay(1000);
}

void connectWiFi() {
  Serial.print("\nMenghubungkan ke WiFi ");
  Serial.print(ssid);
  Serial.print("...");

  WiFi.begin(ssid, password);
  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi terhubung!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nGagal terhubung WiFi!");
  }
}

void checkACState(ACState &ac) {
  // Cek status ON/OFF
  String path = ac.path + "/isOn";
  if (Firebase.getBool(fbData, path)) {
    if (fbData.dataType() == "boolean") {
      bool currentState = fbData.boolData();
      if (currentState != ac.lastState) {
        unsigned long sendMillis = millis();
        Serial.printf("\n[%s] Status berubah: %s -> %s [at %lums]\n", 
                      ac.name.c_str(), 
                      ac.lastState ? "ON" : "OFF", 
                      currentState ? "ON" : "OFF",
                      sendMillis);

        if (sendToServer(ac.path.substring(9) + ":" + (currentState ? "ON" : "OFF"), sendMillis, ac.serverIP, ac.serverPort)) {
          ac.lastState = currentState;
        }
      }
    }
  } else {
    Serial.printf("\n[ERROR] Gagal baca %s: %s\n", path.c_str(), fbData.errorReason().c_str());
  }

  // Cek temperatur
  path = ac.path + "/temperature";
  if (Firebase.getInt(fbData, path)) {
    if (fbData.dataType() == "int") {
      int currentTemp = fbData.intData();
      if (currentTemp != ac.lastTemp) {
        unsigned long sendMillis = millis();
        Serial.printf("\n[%s] Suhu berubah: %d°C -> %d°C [at %lums]\n", 
                      ac.name.c_str(), ac.lastTemp, currentTemp, sendMillis);

        if (sendToServer(ac.path.substring(9) + ":TEMP:" + String(currentTemp), sendMillis, ac.serverIP, ac.serverPort)) {
          ac.lastTemp = currentTemp;
        }
      }
    }
  } else {
    Serial.printf("\n[ERROR] Gagal baca %s: %s\n", path.c_str(), fbData.errorReason().c_str());
  }
}

bool sendToServer(String message, unsigned long sendMillis, IPAddress serverIP, uint16_t serverPort) {
  Serial.print("[NETWORK] Mengirim ke server: ");
  Serial.println(message);
  Serial.printf("[INFO] Waktu kirim: %lums | RSSI WiFi: %d dBm\n", sendMillis, WiFi.RSSI());

  // Kirim ke TCP Server (opsional)
  unsigned long startMillis = millis();
  while (!client.connect(serverIP, serverPort) && (millis() - startMillis < 5000))

  if (!client.connected()) {
    Serial.println("\n[ERROR] Gagal terkoneksi ke server!");
    return false;
  }

  client.println(message);
  client.flush();

  unsigned long responseStart = millis();
  while (client.available() == 0 && (millis() - responseStart < 3000)) {
    delay(100);
  }

  if (client.available()) {
    String response = client.readStringUntil('\n');
    Serial.print("[SERVER] Respons: ");
    Serial.println(response);

    unsigned long receiveMillis = millis();
    Serial.printf("[INFO] Waktu terima: %lums setelah kirim\n", receiveMillis - sendMillis);
  }

  client.stop();
  return true;
}
