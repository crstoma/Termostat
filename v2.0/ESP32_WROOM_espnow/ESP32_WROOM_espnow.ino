#include <WifiEspNow.h>
#include <WiFi.h>
#include <WebServer.h>

#define RELAY_PIN 5  // Pinul pe care este conectat releul

// MAC Address of the sender ESP8266
static uint8_t PEER[]{0x82, 0x7D, 0x3A, 0x2C, 0x32, 0xB8}; // Adresa MAC a ESP8266

// Variabila pentru a stoca temperatura setată
int setTemperature = 25; // Temperatura setată implicit
float currentTemperature = 0.0; // Temperatura curentă

WebServer server(80); // Crearea serverului web pe portul 80

// Funcția pentru a primi mesajele de la ESP8266
void printReceivedMessage(const uint8_t mac[WIFIESPNOW_ALEN], const uint8_t* buf, size_t count, void* arg) {
    Serial.printf("Message from %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    // Presupunem că mesajul este un JSON de forma {"temp": 23.7}
    char message[count + 1]; // Buffer pentru mesaj
    memcpy(message, buf, count); // Copiere mesaj în buffer
    message[count] = '\0'; // Terminare string
    sscanf(message, "{ \"temp\": %f", &currentTemperature); // Extrage temperatura
    Serial.printf("Temperatura primită: %.2f °C\n", currentTemperature);
    
    // Controlul releului în funcție de temperatura citită
    if (currentTemperature < setTemperature) { // Inversarea logicii releului
        digitalWrite(RELAY_PIN, HIGH); // Activare releu
        Serial.println("Releu activat!");
    } else {
        digitalWrite(RELAY_PIN, LOW); // Dezactivare releu
        Serial.println("Releu dezactivat!");
    }
}

// Funcția pentru a gestiona cererea de pe pagina principală
void handleRoot() {
    String html = "<h1>Control Releu</h1><form action=\"/set\" method=\"GET\">"
                  "Setează temperatura: <input type=\"text\" name=\"temp\"><input type=\"submit\" value=\"Setează\"></form>"
                  "<p>Temperatura curentă: " + String(currentTemperature) + " °C</p>"
                  "<p>Temperatura setată: " + String(setTemperature) + " °C</p>";
    server.send(200, "text/html", html);
}

// Funcția pentru a seta temperatura
void handleSet() {
    if (server.hasArg("temp")) {
        setTemperature = server.arg("temp").toInt(); // Setează temperatura dorită
        Serial.printf("Temperatura setată: %d°C\n", setTemperature);
    }
    server.send(200, "text/html", "<h1>Temperatura a fost setată!</h1><a href=\"/\">Înapoi</a>");
}

void setup() {
    Serial.begin(115200);
    Serial.println();

    WiFi.persistent(false);
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32_Server", nullptr, 3);
    WiFi.softAPdisconnect(false);

    Serial.print("MAC address of this node is ");
    Serial.println(WiFi.softAPmacAddress());

    pinMode(RELAY_PIN, OUTPUT); // Setarea pinului releului ca ieșire

    bool ok = WifiEspNow.begin();
    if (!ok) {
        Serial.println("WifiEspNow.begin() failed");
        ESP.restart();
    }

    WifiEspNow.onReceive(printReceivedMessage, nullptr);
    
    ok = WifiEspNow.addPeer(PEER);
    if (!ok) {
        Serial.println("WifiEspNow.addPeer() failed");
        ESP.restart();
    }

    // Configurarea serverului
    server.on("/", handleRoot);
    server.on("/set", handleSet);
    server.begin(); // Pornirea serverului
    Serial.println("Serverul a pornit.");
}

void loop() {
    server.handleClient(); // Procesarea cererilor clientului
    delay(100); // Mică întârziere pentru a evita supraîncărcarea procesorului
}
