#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Configurare WiFi
const char* ssid = "wifi";
const char* password = "password";

// Configurare pin releu
#define RELAY_PIN D1

// Configurare IP static pentru centrala de comandă
IPAddress local_ip(192, 168, 0, 100);   // Adresa IP statică
IPAddress gateway(192, 168, 0, 1);      // Gateway-ul (de obicei, routerul)
IPAddress subnet(255, 255, 255, 0);     // Mască de subrețea

// Setări pentru server
ESP8266WebServer server(80);  // Crează server HTTP pe portul 80

// Variabile pentru controlul centralei
bool heating = false; // Starea centralei (pornită/oprită)

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Configurare pin releu
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // Asigură-te că releul este oprit la început
  
  // Conectare la WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectat la WiFi!");

  // Configurare IP static
  WiFi.config(local_ip, gateway, subnet);
  Serial.print("IP-ul centralei este: ");
  Serial.println(WiFi.localIP());

  // Adaugă o rută HTTP pentru a obține starea curentă
  server.on("/status", HTTP_GET, handleStatusRequest);

  // Configurare rute HTTP
  server.on("/control", HTTP_GET, handleControlRequest);

  // Pornire server
  server.begin();
  Serial.println("Server pornit!");
}

void loop() {
  // Servește cererile HTTP
  server.handleClient();
}

// Funcție pentru a răspunde la cererile de status
void handleStatusRequest() {
  String response = heating ? "Centrala este pornita" : "Centrala este oprita";
  server.send(200, "text/plain", response);
}

// Funcție pentru a gestiona cererea de la termostat
void handleControlRequest() {
  String id = server.arg("id");
  String state = server.arg("state");

  // Verifică ID-ul termostatului și starea
  Serial.println("Cerere primită de la termostat:");
  Serial.print("ID: ");
  Serial.println(id);
  Serial.print("Stare: ");
  Serial.println(state);

  if (id == "1") {
    if (state == "on") {
      heating = true;
      digitalWrite(RELAY_PIN, LOW);  // Pornește centrala
      Serial.println("Centrala pornită!");
    } else if (state == "off") {
      heating = false;
      digitalWrite(RELAY_PIN, HIGH);   // Oprește centrala
      Serial.println("Centrala oprită!");
    }
  }

  // Răspuns HTTP
  server.send(200, "text/plain", heating ? "Centrala este pornita" : "Centrala este oprita");
}
