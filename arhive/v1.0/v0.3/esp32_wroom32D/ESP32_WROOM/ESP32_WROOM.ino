#include <WiFi.h>

// Configurare Wi-Fi
const char* ssid = "wifi"; // Numele rețelei Wi-Fi
const char* password = "password";  // Parola rețelei Wi-Fi

// Configurare IP static
IPAddress local_IP(192, 168, 0, 150);    // Adresa IP statică
IPAddress gateway(192, 168, 0, 1);       // Gateway-ul rețelei (de obicei router-ul)
IPAddress subnet(255, 255, 255, 0);      // Subnetul (masca de rețea)

WiFiServer server(81);

// Pinul pentru controlul releului
const int relayC1 = 25; //Camera 1
const int relayC2 = 17; //Camera 2
const int relayC3 = 16; //Camera 3
const int relayC4 = 24; //Baie
const int relayC5 = 14; //Hol
const int relayC6 = 12; //Bucatarie
//const int relayC7 = 13; //Centrala
//const int relayC8 = 5; //Pompa Aditionala

// Starea curentă a releului
bool relayState = false; // Releul este oprit la început

void setup() {
  Serial.begin(115200);

  // Configurare IP statică
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Configurarea IP-ului static a eșuat");
  }

  // Conectare la rețeaua Wi-Fi
  Serial.print("Conectare la ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Conectat la rețeaua Wi-Fi");
  
  // Pornire server
  server.begin();
  Serial.print("Server is at ");
  Serial.println(WiFi.localIP());

  // Configurare pin releu
  pinMode(relayC1, OUTPUT);
  digitalWrite(relayC1, LOW); // Releul este inițial oprit
}

void loop() {
  WiFiClient client = server.available();
  
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();
    
    // Verifică comenzile pentru Camera 1
    if (request.startsWith("releu=1")) {
      relayState = true; // Setează starea releului la ON
      digitalWrite(relayC1, HIGH); // Releu ON
      Serial.println("Releu Camera 1 activat.");
    } 
    else if (request.startsWith("releu=0")) {
      relayState = false; // Setează starea releului la OFF
      digitalWrite(relayC1, LOW); // Releu OFF
      Serial.println("Releu Camera 1 dezactivat.");
    }
    
    // Verifică comenzile pentru Camera 2
    if (request.startsWith("releu2=1")) {
      digitalWrite(relayC2, HIGH); // Releu ON
      Serial.println("Releu Camera 2 activat.");
    } 
    else if (request.startsWith("releu2=0")) {
      digitalWrite(relayC2, LOW); // Releu OFF
      Serial.println("Releu Camera 2 dezactivat.");
    }

    // Închide conexiunea clientului
    client.stop();
  }
}