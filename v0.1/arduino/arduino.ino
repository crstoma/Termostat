#include <SPI.h>
#include <Ethernet.h>

// Configurare rețea
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // Adresa MAC
IPAddress ip(192, 168, 0, 150); // IP-ul Arduino-ului

EthernetServer server(81);

// Pinul pentru controlul releului
const int relayPin = 7;
float temperatureThreshold = 27.0; // Pragul de temperatură pentru activarea releului

void setup() {
  Serial.begin(9600);
  
  // Pornire Ethernet
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("Server is at ");
  Serial.println(Ethernet.localIP());
  
  // Configurare pin releu
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Releul este inițial oprit
}

void loop() {
  EthernetClient client = server.available();
  
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();
    
    // Extrage temperatura din mesaj
    if (request.startsWith("Temp=")) {
      float receivedTemp = request.substring(5).toFloat();
      Serial.print("Temperature received: ");
      Serial.println(receivedTemp);
      
      // Activează releul dacă temperatura depășește pragul
      if (receivedTemp > temperatureThreshold) {
        digitalWrite(relayPin, HIGH); // Releu ON
      } else {
        digitalWrite(relayPin, LOW); // Releu OFF
      }
    }
  }
}
