#include <SPI.h>
#include <ESP8266WiFi.h>    // menambahkan modul WiFi
#include <ESP8266HTTPClient.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

//------------------------------------------------------------------------------------
// Definisikan pin input output
//------------------------------------------------------------------------------------
#define IR_LED_PIN D2  // D2 untuk IR transmitter (KY-005)
IRsend irsend(IR_LED_PIN);

//------------------------------------------------------------------------------------
// WIFI Autentikasi
//------------------------------------------------------------------------------------
char ssid[] = "08_";                   // nama WiFi
char pass[] = "12345678";               // password WiFi

//------------------------------------------------------------------------------------
// WIFI Module Mode & IP
//------------------------------------------------------------------------------------
IPAddress server(192,168,217,100);           // IP address server
WiFiClient client;

//====================================================================================
void setup() {
                // Inisialisasi Onboard Output Led
  Serial.begin(9600);                       // 
  WiFi.begin(ssid, pass);                   // Menghubungkan ke WiFi
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LedBoard, LOW);
    Serial.print(".");
    delay(500);
  }
  Serial.println("Connected to wifi");
  Serial.print("Status: "); Serial.println(WiFi.status());    // Network parameters
  Serial.print("IP: ");     Serial.println(WiFi.localIP());
  Serial.print("Subnet: "); Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: "); Serial.println(WiFi.gatewayIP());
  Serial.print("SSID: "); Serial.println(WiFi.SSID());
  Serial.print("Signal: "); Serial.println(WiFi.RSSI());
  irsend.begin();
  
}
//====================================================================================
void loop() {
  ContinuousConnection();
}

//====================================================================================

 void ContinuousConnection(){
  client.connect(server, 80);                       // Connection to the server
  digitalWrite(LedBoard,HIGH);
  ReadButton();                                     // Read Button from Transmitter
 }
//====================================================================================

void ReadButton() {
  int reading = digitalRead(BUTTON_1);
  if (reading == LOW) {
    digitalWrite(LedBoard, LOW);                     // to show the communication only (inverted logic)
    client.print("I am Transmitter\r");
    delay(200);
    digitalWrite(LedBoard,HIGH);                     // to show the communication only (inverted logic)
    kirimnotifikasi("Panic Button", "Ruang A");     //kirim notifikasi
  }else{
   ClientContinue(); 
  } 
}

//====================================================================================
void ClientContinue(){
  client.println("Transmmiter");                      // sends the message to the server
  String answer = client.readStringUntil('\r');       // receives the answer from the sever
  client.flush();
  
  if (answer == "I am Receiver") {                    // compares if the response of the receiver is equal to 'SWITCH'
    digitalWrite(Buzzer1, !digitalRead(Buzzer1));           // if it changes the status of the LED
    digitalWrite(LedBoard, LOW);
    Serial.println("Data Received: " + answer);
    delay(200);                                       // client will trigger the communication 200 milliseconds
    digitalWrite(LedBoard, HIGH);
  }
}

void kirimnotifikasi (String judul, String isi)
{
  //ubah nilai spasi menjadi %20
  judul.replace(" ", "%20");
  isi.replace(" ", "%20");

  //cek koneksi ke server

  WiFiClient client;
  {
    Serial.println("Connection Failed");
    return; //coba koneksi lagi
  }
  //proses pengiriman data ke server untuk trigger notifikasi
  String Link;
  Link = "http://192,168,0,101/notifikasi/kirimnotifikasi.php?judul=" + judul + "&isi=" + isi;
  //eksekusi link
  HTTPClient http;
  http.begin(Link);
  //method GET
  http.GET();
  http.end();

  delay(1000);
}
//====================================================================================