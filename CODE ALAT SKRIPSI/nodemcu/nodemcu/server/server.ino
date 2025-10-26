#include <SPI.h>
#include <ESP8266WiFi.h>    // The Basic Function Of The ESP NOD MCU

//------------------------------------------------------------------------------------
// WIFI Module Config
//------------------------------------------------------------------------------------
char ssid[] = "08_";                  // SSID of your home WiFi
char pass[] = "12345678";              // password of your home WiFi

WiFiServer server(80);

IPAddress ip(192,168,217,100);           // IP address of the server
IPAddress gateway(192,168,217,1);        // gateway of your network
IPAddress subnet(255, 255, 255, 0);        // subnet mask of your network

//------------------------------------------------------------------------------------
// Defining I/O Pins
//------------------------------------------------------------------------------------
#define       LedBoard   2        // WIFI Module LED
#define       LED1      D0        // LED Receiver One
#define       SWITCH    D1        // Button

//====================================================================================
void setup() {
  Serial.begin(9600);                   // only for debug
  WiFi.config(ip, gateway, subnet);       // forces to use the fix IP
  WiFi.begin(ssid, pass);                 // connects to the WiFi router
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  server.begin();                         // starts the server
  Serial.println("Connected to wifi");
  Serial.print("Status: ");   Serial.println(WiFi.status());        // some parameters from the network
  Serial.print("IP: ");       Serial.println(WiFi.localIP());
  Serial.print("Subnet: ");   Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: ");  Serial.println(WiFi.gatewayIP());
  Serial.print("SSID: ");     Serial.println(WiFi.SSID());
  Serial.print("Signal: ");   Serial.println(WiFi.RSSI());
  Serial.print("Networks: "); Serial.println(WiFi.scanNetworks());

  pinMode(LedBoard, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(SWITCH, INPUT_PULLUP);
  digitalWrite(LED1, LOW);
}

void loop() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  digitalWrite(LedBoard, HIGH);
  String request = client.readStringUntil('\r');
  client.flush();

  if (request == "I am Transmitter") {
    digitalWrite(LedBoard, LOW);
    digitalWrite(LED1, !digitalRead(LED1));
    Serial.print("Data Received: "); Serial.println(request);
    delay(200);
    digitalWrite(LedBoard, HIGH);
  }

  int reading = digitalRead(SWITCH);
  if (reading == LOW) {
    digitalWrite(LedBoard, LOW);
    client.print("I am Receiver\r");
    delay(200);
    digitalWrite(LedBoard, LOW);
  }
  client.println("Receiver\r");      // sends the answer to the client
  digitalWrite(LedBoard, HIGH);
  delay(100);
}