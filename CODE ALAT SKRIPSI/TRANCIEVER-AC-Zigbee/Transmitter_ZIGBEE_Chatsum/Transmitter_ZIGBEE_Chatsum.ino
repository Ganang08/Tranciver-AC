#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <SoftwareSerial.h>

// ========== KONFIGURASI JARINGAN ==========
const char* ssid = "08_";
const char* password = "12345678";

// ========== KONFIGURASI FIREBASE ==========
#define FIREBASE_HOST "ac-app-b5549-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "wq9RwYVcTNOEN1qDuZWaYwlyvUQHl8pRvlj5jZKr"

// ========== OBJEK ==========
FirebaseData fbData;
FirebaseAuth fbAuth;
FirebaseConfig fbConfig;
WiFiClient client;

// ========== ZIGBEE SERIAL ==========
SoftwareSerial zigbeeSerial(D6, D7); // RX, TX
const long ZIGBEE_BAUD = 9600;

// ========== STRUKTUR DATA AC ==========
struct ACState {
  bool lastState;
  int lastTemp;
  String path;
  String name;
  IPAddress serverIP;
  uint16_t serverPort;
};

ACState acs[] = {
  {false, 0, "/devices/AC_01", "Daikin 1pk", IPAddress(192, 168, 135, 77), 5000}
};
const int AC_COUNT = sizeof(acs) / sizeof(acs[0]);

// ====== SETUP ======
void setup() {
  Serial.begin(115200);
  zigbeeSerial.begin(ZIGBEE_BAUD);

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
    Serial.println("\nGagal koneksi ke Firebase!");
    Serial.println("Error: " + fbData.errorReason());
    while (1) delay(1000);
  }

  Serial.println("\nTerhubung ke Firebase!");
}

// ====== LOOP ======
void loop() {
  // Cek perintah dari Serial untuk unit test
  if (Serial.available()) {
    char input = Serial.read();
    if (input == 't') {
      sendTestCommand("AC_01:ON");
    }
  }

  if (!Firebase.ready() || WiFi.status() != WL_CONNECTED) {
    Serial.println("[ERROR] Koneksi terputus. Reconnecting...");
    connectWiFi();
    delay(5000);
    return;
  }

  for (int i = 0; i < AC_COUNT; i++) {
    checkACState(acs[i]);
    delay(200);
  }

 
}

// ====== WIFI ======
void connectWiFi() {
  Serial.print("\nMenghubungkan ke WiFi ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi terhubung!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nGagal konek WiFi.");
  }
}

// ====== CHECK FIREBASE ======
void checkACState(ACState &ac) {
  // --- ON/OFF
  String path = ac.path + "/isOn";
  if (Firebase.getBool(fbData, path)) {
    if (fbData.dataType() == "boolean") {
      bool currentState = fbData.boolData();
      if (currentState != ac.lastState) {
        unsigned long sendMillis = millis();
        Serial.printf("\n[%s] Status: %s -> %s\n", 
                      ac.name.c_str(), 
                      ac.lastState ? "ON" : "OFF", 
                      currentState ? "ON" : "OFF");

        if (sendToServer(ac.path.substring(9) + ":" + (currentState ? "ON" : "OFF"), sendMillis, ac.serverIP, ac.serverPort)) {
          ac.lastState = currentState;
        }
      }
    }
  }

  // --- Temperature
  path = ac.path + "/temperature";
  if (Firebase.getInt(fbData, path)) {
    if (fbData.dataType() == "int") {
      int currentTemp = fbData.intData();
      if (currentTemp != ac.lastTemp) {
        unsigned long sendMillis = millis();
        Serial.printf("\n[%s] Suhu: %d°C -> %d°C\n", ac.name.c_str(), ac.lastTemp, currentTemp);

        if (sendToServer(ac.path.substring(9) + ":TEMP:" + String(currentTemp), sendMillis, ac.serverIP, ac.serverPort)) {
          ac.lastTemp = currentTemp;
        }
      }
    }
  }
}

// ====== CHECKSUM ======
String addChecksum(String message) {
  byte checksum = 0;
  for (int i = 0; i < message.length(); i++) {
    checksum ^= message[i];
  }
  return message + "|" + String(checksum);
}

// ====== KIRIM PERINTAH ======
bool sendToServer(String message, unsigned long sendMillis, IPAddress serverIP, uint16_t serverPort) {
  String withChecksum = addChecksum(message);
  Serial.println("[ZIGBEE] Mengirim: " + withChecksum);
  zigbeeSerial.println(withChecksum);

  // (TCP server opsional, bisa dipakai jika diperlukan)
  unsigned long startMillis = millis();
  while (!client.connect(serverIP, serverPort) && (millis() - startMillis < 5000));

  if (!client.connected()) {
    Serial.println("[ERROR] TCP server tidak terhubung.");
    return false;
  }

  client.println(message);
  client.flush();

  unsigned long responseStart = millis();
  while (client.available() == 0 && (millis() - responseStart < 3000)) delay(100);

  if (client.available()) {
    String response = client.readStringUntil('\n');
    Serial.println("[SERVER] Respons: " + response);
    unsigned long receiveMillis = millis();
    Serial.printf("[INFO] Waktu respons: %lums\n", receiveMillis - sendMillis);
  }

  client.stop();
  return true;
}

// ====== UNIT TEST ======
void sendTestCommand(String testMessage) {
  String withChecksum = addChecksum(testMessage);
  Serial.println("[TEST] Kirim perintah uji Zigbee: " + withChecksum);
  zigbeeSerial.println(withChecksum);
}
