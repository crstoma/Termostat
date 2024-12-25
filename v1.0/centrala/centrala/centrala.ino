#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Configurare WiFi
const char* ssid = "WIRELESS";
const char* password = "PASSWORD";

// Configurare IP static
IPAddress staticIP(192, 168, 0, 202);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(8080);

// Pinuri pentru relee
#define RELAY1 D1
#define RELAY2 D2

bool boilerState = false;

void setup() {
    Serial.begin(115200);

    if (!WiFi.config(staticIP, gateway, subnet)) {
        Serial.println("Eroare la configurarea IP-ului static!");
    }

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConectat la WiFi!");

    pinMode(RELAY1, OUTPUT);
    pinMode(RELAY2, OUTPUT);
    digitalWrite(RELAY1, HIGH);
    digitalWrite(RELAY2, HIGH);

    server.on("/boiler", handleBoiler);
    server.on("/status", handleStatus);
    server.begin();
}

void loop() {
    server.handleClient();
}

void handleBoiler() {
    String state = server.arg("state");

    if (state == "on") {
        digitalWrite(RELAY1, LOW);
        digitalWrite(RELAY2, LOW);
        boilerState = true;
        server.send(200, "text/plain", "Boiler ON");
    } else if (state == "off") {
        digitalWrite(RELAY1, HIGH);
        digitalWrite(RELAY2, HIGH);
        boilerState = false;
        server.send(200, "text/plain", "Boiler OFF");
    } else {
        server.send(400, "text/plain", "Comandă invalidă");
    }
}

void handleStatus() {
    server.send(200, "text/plain", boilerState ? "Boiler ON" : "Boiler OFF");
}
