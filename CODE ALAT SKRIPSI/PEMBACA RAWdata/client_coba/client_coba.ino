#include <Arduino.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>

// Pin IR Receiver
const uint16_t kRecvPin = D2;  // D2 pada NodeMCU = GPIO4

// Konfigurasi Serial dan buffer
const uint32_t kBaudRate = 115200;  // Ubah ke 115200 agar lebih cepat dan umum
const uint16_t kCaptureBufferSize = 1024;  // Buffer data IR

#if DECODE_AC
const uint8_t kTimeout = 50;
#else
const uint8_t kTimeout = 15;
#endif

const uint16_t kMinUnknownSize = 12;  // Ukuran minimum sinyal IR tak dikenal
#define LEGACY_TIMING_INFO false  // Tampilkan timing info lama jika ingin

// Objek untuk menerima IR
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;

void setup() {
  // Inisialisasi Serial (tanpa SERIAL_TX_ONLY untuk menghindari error)
  Serial.begin(kBaudRate);
  while (!Serial) delay(50);  // Tunggu sampai Serial siap

  Serial.printf("\n" D_STR_IRRECVDUMP_STARTUP "\n", kRecvPin);

#if DECODE_HASH
  irrecv.setUnknownThreshold(kMinUnknownSize);
#endif

  irrecv.enableIRIn();  // Mulai menerima sinyal IR
  Serial.println("IR Receiver aktif dan siap menerima sinyal...");
}

void loop() {
  if (irrecv.decode(&results)) {
    // Tampilkan waktu penerimaan
    uint32_t now = millis();
    Serial.printf(D_STR_TIMESTAMP " : %06u.%03u\n", now / 1000, now % 1000);

    // Tampilkan peringatan jika buffer penuh
    if (results.overflow)
      Serial.printf(D_WARN_BUFFERFULL "\n", kCaptureBufferSize);

    // Versi library
    Serial.println(D_STR_LIBRARY ": v" _IRREMOTEESP8266_VERSION_ "\n");

    // Deskripsi dasar sinyal IR
    Serial.print(resultToHumanReadableBasic(&results));

    // Jika AC signal, tampilkan deskripsi tambahan
    String description = IRAcUtils::resultAcToString(&results);
    if (description.length())
      Serial.println(description);

#if LEGACY_TIMING_INFO
    Serial.println(D_STR_MESGDESC);
    Serial.println(resultToTimingInfo(&results));
    yield();
#endif

    // Tampilkan kode dalam format yang bisa digunakan di kode Arduino
    Serial.println(resultToSourceCode(&results));
    Serial.println();  // Spasi antar sinyal

    // Selesai proses, siap menerima sinyal berikutnya
    irrecv.resume();
    yield();
  }
}
