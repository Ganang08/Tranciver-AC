#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Electra.h>
#include <SoftwareSerial.h>

// ======= KONFIGURASI JARINGAN =======
const char* ssid = "Yayaya";
const char* password = "082244026023";

// ======= SERVER WIFI =======
WiFiServer server(5000);
WiFiClient client;

// ======= KONFIGURASI IR TRANSMITTER =======
const uint16_t kIrLed = 4;  // D2
IRsend irsend(kIrLed);



// ======= STRUKTUR DATA UNTUK AC =======
struct ACProfile {
  String name;
  const uint16_t* powerOnRaw;
  const uint16_t* powerOffRaw;
  const uint16_t* tempRaw[15]; // suhu 16-30 (index 0-14)
  size_t rawLength;
};

// ======= RAW DATA =======
const uint16_t ac1PowerOn[207] PROGMEM = {4056, 4004,  514, 1962,  536, 1984,  514, 1984,  514, 1984,  514, 964,  534, 964,  534, 1984,  514, 964,  536, 1984,  512, 964,  534, 1982,  514, 966,  534, 964,  534, 964,  534, 964,  534, 964,  534, 1984,  512, 1984,  514, 964,  534, 1982,  514, 964,  534, 1984,  514, 966,  534, 1982,  514, 8610,  4040, 4006,  514, 1984,  512, 1984,  512, 1984,  514, 1984,  514, 964,  534, 964,  534, 1984,  514, 964,  534, 1982,  514, 964,  534, 1984,  514, 964,  534, 964,  534, 964,  534, 964,  534, 966,  532, 1984,  514, 1982,  514, 964,  534, 1984,  514, 964,  534, 1984,  514, 966,  532, 1984,  514, 8636,  4040, 4008,  514, 1982,  516, 1982,  514, 1984,  514, 1982,  516, 982,  514, 964,  534, 1982,  514, 984,  514, 1984,  514, 984,  514, 1982,  514, 984,  514, 984,  514, 984,  514, 984,  514, 984,  514, 1982,  516, 1982,  516, 982,  516, 1982,  516, 984,  514, 1980,  516, 984,  516, 1980,  516, 8610,  4040, 4008,  514, 1964,  534, 1982,  514, 1984,  512, 1984,  514, 984,  514, 984,  514, 1982,  516, 984,  516, 1980,  516, 984,  514, 1982,  516, 982,  514, 984,  514, 984,  514, 984,  514, 984,  514, 1984,  514, 1984,  514, 984,  514, 1984,  512, 988,  512, 1986,  510, 988,  510, 1988,  510}; // Isi sama seperti sebelumnya
const uint16_t ac1PowerOff[207] PROGMEM = {4060, 3988,  534, 1984,  514, 1962,  536, 1982,  514, 1984,  514, 962,  568, 932,  566, 1954,  514, 964,  566, 1954,  514, 962,  566, 1932,  534, 962,  566, 932,  566, 932,  568, 932,  566, 932,  566, 1954,  514, 1984,  514, 962,  568, 1952,  514, 960,  538, 1964,  536, 962,  566, 1930,  564, 8588,  4090, 3956,  564, 1954,  544, 1954,  544, 1954,  544, 1932,  564, 934,  566, 930,  538, 1982,  546, 932,  566, 1932,  566, 932,  566, 1932,  566, 932,  566, 932,  566, 932,  566, 932,  566, 932,  566, 1932,  564, 1932,  566, 932,  566, 1952,  546, 932,  566, 1930,  568, 932,  566, 1930,  566, 8586,  4090, 3956,  566, 1932,  538, 1960,  536, 1962,  536, 1962,  536, 964,  534, 988,  510, 1962,  536, 988,  512, 1960,  536, 988,  510, 1964,  532, 990,  508, 990,  486, 1014,  484, 1014,  486, 1014,  486, 1984,  514, 1984,  540, 960,  540, 1956,  540, 960,  540, 1958,  538, 960,  538, 1958,  538};
const uint16_t ac1Temp17[207] PROGMEM = {4058, 3986,  536, 1960,  568, 1932,  566, 1932,  566, 1932,  564, 934,  566, 1932,  538, 960,  564, 1932,  564, 1934,  536, 962,  566, 932,  566, 1932,  536, 962,  566, 932,  566, 932,  566, 932,  538, 1960,  536, 962,  566, 1932,  536, 962,  566, 932,  566, 1932,  566, 1932,  564, 932,  566, 8586,  4060, 3988,  536, 1962,  564, 1934,  536, 1962,  566, 1932,  536, 962,  566, 1932,  536, 962,  566, 1932,  536, 1962,  540, 958,  540, 958,  542, 1956,  540, 958,  538, 960,  536, 962,  538, 960,  538, 1960,  536, 964,  538, 1958,  538, 960,  538, 962,  538, 1958,  538, 1962,  536, 962,  536, 8642,  4058, 3988,  536, 1960,  536, 1962,  536, 1960,  536, 1962,  536, 962,  534, 1962,  536, 962,  536, 1962,  534, 1964,  510, 988,  512, 986,  512, 1986,  510, 988,  512, 988,  510, 988,  510, 1010,  488, 1984,  540, 982,  518, 1956,  570, 952,  550, 948,  576, 1896,  604, 1892,  606, 918,  578, 8522,  4120, 3926,  596, 1902,  568, 1930,  566, 1930,  566, 1932,  562, 936,  538, 1962,  536, 962,  536, 1962,  536, 1962,  536, 962,  536, 962,  536, 1962,  534, 964,  534, 964,  534, 964,  534, 964,  532, 1986,  512, 986,  510, 1986,  510, 988,  512, 986,  510, 1988,  508, 1988,  510, 988,  510};
const uint16_t ac1Temp18[207] PROGMEM = {4060, 3988,  534, 1960,  538, 1960,  542, 1956,  536, 1962,  536, 1962,  536, 1962,  536, 1960,  566, 1934,  540, 958,  542, 1956,  536, 960,  542, 956,  544, 954,  540, 960,  540, 956,  542, 958,  538, 958,  542, 956,  542, 956,  542, 956,  538, 1958,  540, 958,  542, 1956,  538, 1960,  538, 8694,  4062, 3988,  534, 1960,  542, 1956,  538, 1960,  536, 1960,  536, 1960,  538, 1960,  540, 1956,  542, 1956,  536, 962,  542, 1954,  538, 962,  538, 960,  542, 956,  538, 960,  540, 958,  538, 960,  538, 960,  538, 960,  540, 958,  538, 960,  538, 1962,  534, 962,  536, 1960,  534, 1964,  534, 8616,  4036, 4012,  510, 1986,  512, 1986,  510, 1986,  512, 1986,  512, 1986,  512, 1986,  512, 1986,  512, 1984,  538, 988,  514, 1958,  568, 954,  548, 950,  574, 924,  578, 920,  578, 920,  580, 918,  578, 922,  576, 922,  574, 926,  572, 902,  594, 1902,  570, 930,  566, 1932,  564, 1932,  566, 8612,  4060, 3988,  534, 1962,  534, 1964,  534, 1962,  534, 1964,  532, 1964,  534, 1964,  534, 1964,  534, 1964,  532, 966,  534, 1964,  532, 986,  512, 986,  510, 988,  510, 988,  510, 988,  510, 988,  510, 988,  510, 990,  508, 990,  508, 990,  508, 1990,  508, 992,  506, 1992,  506, 1992,  504};

// ======= RAW DATA =======
const uint16_t ac2PowerOn[] PROGMEM = {};
const uint16_t ac2PowerOff[] PROGMEM = {};
const uint16_t ac2Temp17[] PROGMEM = {};
const uint16_t ac2Temp18[] PROGMEM = {};
const uint16_t ac2Temp19[] PROGMEM = {};
// ======= RAW DATA =======
const uint16_t ac3PowerOn[] PROGMEM = {};
const uint16_t ac3PowerOff[] PROGMEM = {};
const uint16_t ac3Temp17[] PROGMEM = {};
const uint16_t ac3Temp18[] PROGMEM = {};
const uint16_t ac3Temp19[] PROGMEM = {};
// ======= PROFIL AC =======
ACProfile acProfiles[] = {
  {
    "Daikin 1pk",
    ac1PowerOn,
    ac1PowerOff,
    {},
    sizeof(ac1PowerOn) / sizeof(uint16_t)
  },
  {
    "Daikin 1.5pk",
    ac2PowerOn,
    ac2PowerOff,
    {},
    sizeof(ac2PowerOn) / sizeof(uint16_t)
  },
  {
    "Daikin 2pk",
    ac3PowerOn,
    ac3PowerOff,
    {},
    sizeof(ac3PowerOn) / sizeof(uint16_t)
  }
};
const int AC_COUNT = sizeof(acProfiles) / sizeof(acProfiles[0]);

// ======= SETUP =======
void setup() {
  Serial.begin(115200);
  irsend.begin();
  
  connectWiFi();
  server.begin();

  Serial.println("\nServer siap!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  for (int i = 0; i < AC_COUNT; i++) {
    Serial.printf("- AC_%02d: %s\n", i + 1, acProfiles[i].name.c_str());
  }
}

// ======= LOOP =======
void loop() {
  if (!client || !client.connected()) {
    client = server.available();
    if (client) {
      Serial.println("\nClient TCP terhubung: " + client.remoteIP().toString());
    }
  }

  if (client && client.available()) {
    String command = client.readStringUntil('\n');
    command.trim();
    Serial.println("[WiFi] Perintah: " + command);
    processCommand(command, "WiFi");
  }

}

// ======= KONEKSI WIFI =======
void connectWiFi() {
  Serial.print("Menghubungkan ke WiFi ");
  Serial.print(ssid);
  Serial.println("...");
  WiFi.begin(ssid, password);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.status() == WL_CONNECTED ? "\nWiFi Terhubung!" : "\nGagal konek WiFi.");
}

// ======= KIRIM SINYAL IR (dengan pembacaan dari PROGMEM) =======
void sendIR(const uint16_t* rawData, size_t length) {
  uint16_t* tempData = new uint16_t[length];
  for (size_t i = 0; i < length; i++) {
    tempData[i] = pgm_read_word_near(rawData + i);
  }

  for (int i = 0; i < 3; i++) {
    irsend.sendRaw(tempData, length, 38);  // 38kHz
    delay(100);
  }

  delete[] tempData;
  Serial.println("Sinyal IR dikirim");
}

// ======= PROSES PERINTAH =======
void processCommand(String command, String source) {
  unsigned long timeReceived = millis();
  long sentTimestamp = 0;

  int firstSep = command.indexOf(':');
  int secondSep = command.indexOf(':', firstSep + 1);

  String device, action;
  if (secondSep != -1) {
    device = command.substring(0, firstSep);
    action = command.substring(firstSep + 1, secondSep);
    sentTimestamp = command.substring(secondSep + 1).toInt();
  } else {
    device = command.substring(0, firstSep);
    action = command.substring(firstSep + 1);
  }

  if (!device.startsWith("AC_")) {
    Serial.println("[ERROR] Nama perangkat salah");
    return;
  }

  int acIndex = device.substring(3).toInt() - 1;
  if (acIndex < 0 || acIndex >= AC_COUNT) {
    Serial.println("[ERROR] Nomor AC tidak valid");
    return;
  }

  ACProfile& ac = acProfiles[acIndex];

  if (action == "ON") {
    sendIR(ac.powerOnRaw, ac.rawLength);
  } else if (action == "OFF") {
    sendIR(ac.powerOffRaw, ac.rawLength);
  } else if (action.startsWith("TEMP:")) {
    int temp = action.substring(5).toInt();
    if (temp < 16 || temp > 30) {
      Serial.println("[ERROR] Suhu di luar batas");
      return;
    }
    int tempIndex = temp - 16;
    if (ac.tempRaw[tempIndex] == nullptr) {
      Serial.println("[ERROR] Data suhu tidak tersedia");
      return;
    }
    sendIR(ac.tempRaw[tempIndex], ac.rawLength);
  } else {
    Serial.println("[ERROR] Perintah tidak dikenali");
    return;
  }

  unsigned long delayMs = (sentTimestamp > 0) ? timeReceived - sentTimestamp : 0;
  int rssi = WiFi.RSSI();

  Serial.printf("[INFO] Sumber: %s\n", source.c_str());
  Serial.printf("[INFO] AC: %s | Aksi: %s\n", ac.name.c_str(), action.c_str());
  Serial.printf("[INFO] Waktu Diterima: %lu ms\n", timeReceived);
  if (sentTimestamp > 0) {
    Serial.printf("[INFO] Timestamp Kirim: %ld ms\n", sentTimestamp);
    Serial.printf("[INFO] Delay: %lu ms\n", delayMs);
  } else {
    Serial.println("[INFO] Timestamp Kirim tidak tersedia");
  }
  if (source == "WiFi") {
    Serial.printf("[INFO] WiFi RSSI: %d dBm\n", rssi);
  }

  String response = "ACK:" + device + ":" + action + ":Delay=" + String(delayMs) + "ms";
  if (source == "WiFi" && client) {
    client.println(response);
  } 
}
