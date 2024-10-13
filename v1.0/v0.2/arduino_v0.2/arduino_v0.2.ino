#include <SPI.h>
#include <Ethernet.h>

// Configurare rețea
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // Adresa MAC
IPAddress ip(192, 168, 0, 150); // IP-ul Arduino-ului

EthernetServer server(81);

// Pinul pentru controlul releului
const int relayC1 = 2; //Camera 1
const int relayC2 = 3; //Camera 2
const int relayC3 = 4; //Camera 3
const int relayC4 = 5; //Baie
const int relayC5 = 6; //Hol
const int relayC6 = 7; //Bucatarie
//const int relayC7 = 8; //Centrala
//const int relayC8 = 9; //Pompa Aditionala

// Starea curentă a releului
bool relayState = false; // Releul este oprit la început

void setup() {
  Serial.begin(9600);
  
  // Pornire Ethernet
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("Server is at ");
  Serial.println(Ethernet.localIP());
  
  // Configurare pin releu
  pinMode(relayC1, OUTPUT);
  digitalWrite(relayC1, LOW); // Releul este inițial oprit
}

void loop() {
  EthernetClient client = server.available();
  
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();
    
    // Verifică comenzile pentru Camera 1
    if (request.startsWith("releu=")) {
      // Extrage starea din cerere
      String state = request.substring(6);
      Serial.print("Received command: ");
      Serial.println(state);
      
      if (state == "1") {
        relayState = true; // Setează starea releului la ON
        digitalWrite(relayC1, HIGH); // Releu ON
        Serial.println("Releu activat.");
      } 
      else if (state == "0") {
        // Nu modificăm starea releului aici
        Serial.println("Releu rămâne OFF.");
      } 
      else {
        Serial.println("Comandă invalidă. Folosește 0 sau 1.");
      }
    }
    
        // Verifică comenzile pentru Camera 2
    if (request.startsWith("releu=")) {
      // Extrage starea din cerere
      String state = request.substring(6);
      Serial.print("Received command: ");
      Serial.println(state);
      
      if (state == "1") {
        relayState = true; // Setează starea releului la ON
        digitalWrite(relayC2, HIGH); // Releu ON
        Serial.println("Releu activat.");
      } 
      else if (state == "0") {
        // Nu modificăm starea releului aici
        Serial.println("Releu rămâne OFF.");
      } 
      else {
        Serial.println("Comandă invalidă. Folosește 0 sau 1.");
      }
    }

    // Închide conexiunea clientului
    client.stop();
  }
}
