#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <SoftwareSerial.h>

const char* ssid = "08_";
const char* password = "12345678";
WiFiServer server(5000);
WiFiClient client;

const uint16_t kIrLed = 4;  // D2
IRsend irsend(kIrLed);
SoftwareSerial zigbeeSerial(D6, D7);  // RX, TX

// Perbaikan: tempRaw jadi pointer ke array pointer
struct ACProfile {
  String name;
  const uint16_t* powerOnRaw;
  const uint16_t* powerOffRaw;
  const uint16_t* const* tempRaw;
  size_t rawLength;
};

// === DATA IR ===
const uint16_t ac1PowerOn[319] PROGMEM = {470, 404,  470, 400,  468, 400,  478, 392,  468, 404,  478, 25368,  3514, 1728,  482, 1238,  468, 400,  468, 398,  468, 402,  468, 1290,  484, 364,  468, 400,  468, 400,  468, 400,  468, 1270,  470, 398,  470, 1292,  446, 1292,  446, 402,  468, 1290,  446, 1292,  448, 1292,  446, 1270,  466, 1294,  448, 398,  468, 402,  468, 1294,  446, 400,  468, 400,  468, 402,  468, 402,  468, 400,  468, 400,  468, 402,  468, 400,  468, 400,  468, 400,  468, 398,  468, 402,  468, 400,  468, 402,  468, 402,  468, 398,  468, 402,  468, 402,  468, 1290,  478, 368,  468, 402,  468, 400,  470, 1290,  448, 1292,  450, 398,  468, 400,  470, 402,  468, 400,  466, 1290,  484, 366,  504, 364,  504, 1254,  448, 400,  504, 394,  476, 390,  476, 394,  452, 416,  454, 416,  452, 416,  452, 416,  452, 414,  452, 418,  454, 1262,  442, 1304,  442, 1288,  444, 1292,  446, 428,  502, 364,  500, 1238,  466, 402,  468, 402,  468, 400,  468, 400,  470, 400,  468, 406,  468, 394,  468, 402,  468, 400,  468, 400,  468, 402,  466, 402,  466, 402,  466, 402,  466, 404,  466, 402,  466, 404,  464, 404,  464, 426,  444, 424,  444, 426,  442, 426,  442, 428,  442, 426,  442, 428,  440, 428,  440, 430,  440, 432,  438, 430,  438, 434,  436, 432,  432, 460,  406, 440,  430, 462,  406, 468,  406, 458,  406, 460,  406, 462,  406, 466,  406, 460,  406, 464,  406, 462,  406, 464,  406, 468,  406, 458,  406, 464,  406, 462,  406, 462,  406, 462,  406, 1332,  406, 466,  404, 1334,  404, 464,  406, 462,  406, 464,  406, 1332,  404, 1334,  406, 464,  406, 472,  404, 456,  404, 462,  406, 466,  404, 464,  406, 464,  404, 464,  402, 468,  404, 466,  402, 494,  370, 1366,  354, 516,  354, 516,  354, 514,  354, 516,  354, 1388,  352, 1384,  354, 514,  352, 524,  352, 508,  352, 516,  352, 544,  326, 1416,  326};
const uint16_t ac1PowerOff[319] PROGMEM = {468, 406,  468, 400,  468, 398,  468, 400,  470, 398,  470, 25384,  3512, 1706,  466, 1272,  466, 400,  468, 400,  468, 400,  468, 1272,  468, 398,  468, 402,  468, 400,  468, 402,  468, 1292,  446, 402,  468, 1270,  466, 1272,  468, 400,  468, 1270,  468, 1272,  468, 1272,  468, 1270,  468, 1290,  452, 394,  468, 400,  468, 1272,  468, 400,  468, 400,  470, 398,  468, 402,  468, 402,  468, 400,  470, 400,  470, 398,  468, 400,  468, 402,  470, 400,  470, 400,  468, 398,  470, 402,  468, 400,  468, 402,  468, 398,  470, 400,  468, 400,  470, 398,  468, 402,  468, 408,  468, 394,  470, 1268,  504, 366,  468, 400,  468, 402,  468, 400,  468, 400,  470, 404,  468, 394,  468, 402,  468, 1270,  506, 1234,  504, 362,  470, 400,  470, 398,  468, 406,  468, 394,  470, 400,  470, 398,  470, 400,  470, 1272,  506, 1232,  504, 1234,  504, 1240,  504, 356,  468, 1272,  504, 362,  468, 1272,  506, 364,  470, 400,  470, 398,  470, 398,  498, 372,  470, 398,  470, 400,  498, 372,  506, 364,  480, 388,  480, 390,  478, 416,  452, 418,  452, 418,  452, 416,  452, 416,  450, 420,  450, 418,  450, 420,  450, 416,  454, 416,  456, 414,  484, 390,  482, 380,  488, 382,  486, 382,  486, 386,  484, 384,  482, 388,  480, 364,  502, 390,  478, 366,  502, 368,  500, 368,  496, 372,  494, 372,  470, 406,  468, 394,  470, 400,  468, 400,  468, 400,  468, 402,  468, 402,  468, 400,  468, 402,  466, 404,  466, 404,  464, 404,  464, 1292,  444, 426,  444, 1294,  444, 426,  442, 424,  444, 428,  442, 1294,  442, 1300,  442, 426,  442, 426,  442, 428,  440, 430,  440, 428,  436, 434,  436, 1302,  438, 434,  432, 458,  406, 462,  408, 462,  406, 1330,  406, 462,  406, 464,  406, 464,  406, 462,  406, 462,  406, 1334,  406, 1330,  406, 1332,  406, 464,  406, 1334,  406, 462,  406, 1340,  406};
const uint16_t ac1Temp18[319] PROGMEM = {504, 368,  504, 346,  500, 386,  482, 392,  442, 426,  480, 25372,  3522, 1694,  470, 1274,  480, 384,  504, 364,  478, 392,  478, 1258,  480, 390,  480, 390,  502, 366,  480, 388,  482, 1256,  470, 396,  480, 1260,  472, 1266,  476, 392,  480, 1260,  504, 1234,  478, 1262,  480, 1258,  470, 1268,  480, 388,  480, 390,  504, 1242,  478, 388,  478, 388,  478, 388,  480, 390,  480, 392,  478, 392,  478, 388,  480, 390,  478, 392,  476, 392,  478, 392,  476, 392,  476, 390,  478, 390,  478, 394,  478, 390,  478, 388,  478, 392,  480, 1260,  478, 392,  478, 390,  478, 392,  480, 1254,  478, 1262,  480, 390,  478, 390,  478, 394,  478, 392,  476, 1262,  478, 388,  478, 392,  476, 1260,  478, 394,  474, 392,  478, 392,  476, 394,  478, 392,  476, 392,  476, 394,  478, 390,  476, 392,  476, 394,  476, 1258,  478, 1262,  478, 1260,  476, 1262,  480, 392,  476, 392,  476, 1262,  480, 392,  480, 390,  474, 394,  478, 390,  478, 390,  478, 418,  452, 418,  454, 412,  458, 412,  482, 386,  484, 384,  486, 382,  512, 364,  512, 352,  514, 356,  512, 354,  514, 356,  514, 356,  512, 356,  512, 356,  512, 358,  510, 360,  508, 360,  508, 358,  508, 364,  504, 366,  480, 394,  480, 386,  478, 390,  476, 392,  476, 370,  494, 384,  466, 394,  494, 372,  472, 396,  498, 374,  470, 398,  468, 400,  468, 404,  468, 396,  470, 404,  468, 400,  468, 400,  468, 400,  468, 402,  468, 400,  468, 402,  466, 424,  444, 426,  444, 1294,  446, 422,  446, 1294,  444, 424,  444, 424,  444, 426,  444, 1294,  444, 1296,  444, 428,  440, 428,  442, 426,  442, 428,  440, 430,  440, 428,  438, 432,  438, 432,  438, 432,  434, 436,  430, 462,  406, 1330,  406, 466,  406, 460,  406, 462,  406, 464,  406, 1332,  406, 1334,  406, 464,  406, 464,  406, 462,  406, 466,  406, 460,  406, 1332,  406};
const uint16_t ac1Temp19[319] PROGMEM = {504, 368,  500, 370,  504, 364,  470, 398,  470, 406,  478, 25370,  3546, 1668,  470, 1270,  504, 364,  468, 400,  472, 398,  468, 1270,  470, 400,  504, 362,  504, 366,  506, 362,  504, 1234,  470, 398,  468, 1270,  468, 1270,  470, 400,  506, 1232,  504, 1232,  504, 1234,  468, 1272,  504, 1234,  504, 364,  504, 366,  504, 1234,  504, 364,  504, 362,  506, 362,  506, 366,  506, 364,  504, 364,  506, 364,  468, 400,  504, 372,  506, 358,  504, 364,  504, 364,  504, 364,  506, 366,  504, 362,  504, 364,  504, 366,  504, 366,  504, 1230,  506, 366,  506, 362,  506, 364,  506, 1234,  508, 1232,  504, 362,  504, 364,  506, 362,  504, 1236,  470, 1268,  504, 364,  502, 368,  502, 1236,  500, 368,  480, 388,  506, 366,  480, 388,  504, 364,  482, 388,  504, 364,  504, 366,  504, 366,  530, 338,  480, 1260,  504, 1236,  480, 1258,  480, 1258,  504, 366,  480, 388,  478, 1260,  468, 400,  480, 390,  480, 390,  502, 366,  480, 390,  478, 392,  502, 364,  478, 390,  478, 390,  478, 390,  478, 392,  478, 392,  476, 418,  450, 420,  450, 416,  450, 418,  452, 422,  478, 384,  458, 410,  486, 384,  488, 382,  514, 354,  514, 354,  514, 356,  512, 358,  512, 358,  512, 358,  512, 358,  508, 358,  486, 382,  484, 384,  482, 388,  480, 388,  478, 390,  478, 370,  498, 366,  500, 370,  498, 368,  498, 372,  498, 370,  496, 372,  470, 400,  468, 406,  470, 392,  470, 400,  468, 402,  468, 398,  468, 400,  468, 404,  466, 1270,  466, 402,  466, 1292,  446, 426,  444, 422,  446, 424,  444, 1296,  444, 1296,  442, 424,  444, 424,  444, 426,  442, 430,  442, 426,  440, 426,  442, 430,  440, 428,  440, 430,  440, 430,  438, 430,  436, 1304,  434, 436,  430, 462,  406, 462,  406, 462,  406, 1330,  406, 464,  406, 1334,  406, 462,  406, 464,  404, 464,  406, 464,  406, 1332,  406};
const uint16_t ac1Temp20[319] PROGMEM = {430, 462,  412, 440,  404, 484,  382, 486,  412, 434,  434, 25444,  3454, 1764,  408, 1326,  412, 460,  410, 458,  410, 458,  438, 1300,  412, 458,  410, 458,  412, 460,  408, 462,  408, 1328,  412, 456,  412, 1304,  434, 1332,  410, 466,  406, 1322,  410, 1358,  382, 1328,  410, 1332,  410, 1324,  412, 458,  410, 462,  406, 1330,  412, 458,  412, 458,  408, 456,  412, 462,  434, 410,  432, 460,  410, 488,  356, 484,  410, 456,  414, 458,  436, 434,  410, 460,  408, 460,  406, 460,  410, 460,  410, 460,  408, 462,  408, 458,  410, 1328,  408, 460,  434, 434,  408, 460,  410, 1330,  408, 1332,  408, 460,  408, 460,  410, 486,  408, 432,  414, 458,  410, 1328,  410, 432,  434, 1330,  388, 484,  432, 432,  408, 460,  408, 460,  410, 460,  434, 434,  412, 458,  410, 460,  408, 458,  410, 462,  408, 1330,  462, 1276,  410, 1330,  408, 1330,  410, 460,  410, 460,  434, 1302,  410, 460,  408, 460,  410, 458,  436, 430,  410, 438,  430, 464,  408, 458,  410, 458,  410, 470,  408, 450,  410, 486,  382, 464,  404, 436,  432, 464,  406, 460,  412, 460,  410, 484,  404, 466,  382, 458,  460, 412,  406, 458,  412, 460,  410, 486,  406, 436,  408, 460,  410, 458,  408, 460,  410, 456,  410, 460,  410, 460,  408, 462,  434, 436,  408, 460,  410, 462,  406, 462,  408, 460,  408, 458,  436, 460,  382, 514,  382, 436,  408, 484,  384, 460,  410, 460,  410, 458,  408, 484,  386, 486,  382, 460,  410, 458,  434, 436,  434, 1302,  410, 462,  408, 1330,  410, 492,  382, 454,  406, 460,  408, 1330,  414, 1326,  412, 458,  414, 456,  408, 460,  406, 466,  406, 458,  412, 458,  432, 436,  410, 460,  410, 460,  432, 434,  410, 460,  410, 1330,  410, 460,  406, 460,  436, 436,  408, 458,  410, 1332,  408, 1332,  406, 1336,  436, 428,  410, 460,  408, 458,  408, 488,  410, 1300,  408};
const uint16_t ac1Temp21[319] PROGMEM = {406, 464,  412, 458,  410, 464,  380, 486,  408, 460,  408, 25444,  3454, 1766,  406, 1330,  408, 460,  408, 460,  410, 458,  410, 1330,  408, 458,  410, 460,  410, 462,  408, 460,  406, 1332,  408, 458,  436, 1302,  412, 1328,  412, 460,  410, 1330,  410, 1328,  408, 1332,  410, 1326,  412, 1330,  410, 460,  408, 458,  410, 1332,  412, 456,  408, 460,  406, 462,  406, 464,  410, 458,  410, 458,  412, 458,  406, 462,  410, 436,  462, 432,  408, 462,  408, 460,  410, 456,  410, 486,  380, 462,  438, 456,  410, 436,  434, 430,  408, 1330,  410, 462,  408, 458,  408, 460,  410, 1358,  354, 1356,  408, 460,  410, 456,  412, 460,  408, 1332,  410, 456,  464, 1276,  406, 466,  406, 1330,  406, 462,  408, 484,  386, 460,  408, 460,  436, 434,  406, 464,  410, 462,  406, 488,  382, 462,  404, 488,  380, 1332,  458, 1308,  378, 1358,  382, 1332,  406, 490,  382, 458,  406, 1358,  384, 460,  382, 512,  380, 464,  406, 462,  410, 456,  410, 486,  358, 488,  404, 464,  410, 484,  380, 464,  410, 458,  408, 460,  410, 458,  410, 458,  408, 488,  384, 460,  408, 468,  410, 452,  408, 512,  354, 488,  382, 488,  384, 484,  408, 462,  382, 512,  356, 460,  410, 462,  408, 484,  382, 460,  408, 460,  410, 488,  384, 486,  380, 484,  384, 490,  408, 434,  406, 460,  408, 462,  408, 486,  356, 510,  382, 486,  384, 486,  384, 458,  408, 460,  410, 514,  330, 484,  408, 492,  384, 480,  386, 488,  382, 456,  412, 458,  408, 1334,  380, 486,  408, 1362,  382, 482,  382, 486,  382, 462,  408, 1356,  410, 1302,  408, 488,  380, 462,  408, 492,  380, 458,  412, 486,  382, 460,  410, 462,  408, 484,  382, 486,  388, 456,  410, 458,  408, 1358,  406, 434,  410, 514,  356, 458,  410, 460,  406, 1358,  382, 488,  382, 458,  406, 1358,  382, 488,  380, 460,  410, 484,  386, 1356,  380};


// Array pointer suhu untuk Daikin 1pk
const uint16_t* const ac1Temps[319] = {
  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, // 16–22
  &ac1Temp18[319],   // 23 (index 7)
  &ac1Temp19[319],   // 24 (index 8)
  &ac1Temp20[319],   // 25 (index 9)
 
  nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr  // 26–30
};

// Dummy data AC lainnya
const uint16_t ac2PowerOn[] PROGMEM = {};
const uint16_t ac2PowerOff[] PROGMEM = {};
const uint16_t ac3PowerOn[] PROGMEM = {};
const uint16_t ac3PowerOff[] PROGMEM = {};

// === Daftar Profil AC ===
ACProfile acProfiles[] = {
  { "Daikin 1pk", ac1PowerOn, ac1PowerOff, ac1Temps, sizeof(ac1PowerOn)/sizeof(uint16_t) },
  { "Daikin 1.5pk", ac2PowerOn, ac2PowerOff, nullptr, sizeof(ac2PowerOn)/sizeof(uint16_t) },
  { "Daikin 2pk", ac3PowerOn, ac3PowerOff, nullptr, sizeof(ac3PowerOn)/sizeof(uint16_t) }
};
const int AC_COUNT = sizeof(acProfiles) / sizeof(acProfiles[0]);

// === Setup ===
void setup() {
  Serial.begin(115200);
  irsend.begin();
  zigbeeSerial.begin(9600);
  connectWiFi();
  server.begin();

  Serial.println("\nServer siap!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  for (int i = 0; i < AC_COUNT; i++) {
    Serial.printf("- AC_%02d: %s\n", i + 1, acProfiles[i].name.c_str());
  }
}

// === Loop ===
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

  if (zigbeeSerial.available()) {
    String incoming = zigbeeSerial.readStringUntil('\n');
    incoming.trim();
    if (incoming.length() > 0) {
      String commandData;
      if (validateZigbeeMessage(incoming, commandData)) {
        Serial.println("[ZIGBEE] Perintah valid: " + commandData);
        processCommand(commandData, "Zigbee");
      } else {
        Serial.println("[ZIGBEE] [ERROR] Format atau checksum tidak valid: " + incoming);
      }
    }
  }
}

// === Koneksi WiFi ===
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

// === Checksum Zigbee ===
uint8_t calculateChecksum(const String& data) {
  uint8_t checksum = 0;
  for (size_t i = 0; i < data.length(); ++i) {
    checksum ^= data[i];
  }
  return checksum;
}

bool validateZigbeeMessage(const String& message, String& commandOut) {
  int sepIndex = message.lastIndexOf(';');
  if (sepIndex == -1) return false;
  String dataPart = message.substring(0, sepIndex);
  String checksumPart = message.substring(sepIndex + 1);
  uint8_t expected = calculateChecksum(dataPart);
  uint8_t received = checksumPart.toInt();
  if (expected == received) {
    commandOut = dataPart;
    return true;
  }
  return false;
}

// === Kirim IR ===
void sendIR(const uint16_t* rawData, size_t length) {
  uint16_t* tempData = new uint16_t[length];
  for (size_t i = 0; i < length; i++) {
    tempData[i] = pgm_read_word_near(rawData + i);
  }
  for (int i = 0; i < 3; i++) {
    irsend.sendRaw(tempData, length, 38);
    delay(100);
  }
  delete[] tempData;
  Serial.println("Sinyal IR dikirim");
}

// === Proses Perintah ===
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
    if (!ac.tempRaw || ac.tempRaw[tempIndex] == nullptr) {
      Serial.println("[ERROR] Data suhu tidak tersedia");
      return;
    }
    sendIR(ac.tempRaw[tempIndex], ac.rawLength);
  } else {
    Serial.println("[ERROR] Perintah tidak dikenali");
    return;
  }

  // Log
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

  // Kirim Balasan
  String response = "ACK:" + device + ":" + action + ":Delay=" + String(delayMs) + "ms";
  if (source == "WiFi" && client) {
    client.println(response);
  } else if (source == "Zigbee") {
    zigbeeSerial.println(response);
  }
}
