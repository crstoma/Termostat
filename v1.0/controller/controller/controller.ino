#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>

// Configurare WiFi
const char* ssid = "WIRELESS";
const char* password = "PASSWORD";

// Configurare IP static
IPAddress staticIP(192, 168, 0, 100);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

// Configurare server
WebServer server(8080);

// Configurare boiler
const char* gasBoilerIP = "192.168.0.202";
const int gasBoilerPort = 8080;

// Configurare zone și actuatoare
const int actuators[6] = {16, 17, 25, 26, 14, 27};
bool heatingRequest[6] = {false};
float temps[6] = {0};
float setTemps[6] = {20, 20, 20, 20, 20, 20};
unsigned long lastUpdate[6] = {0};
unsigned long timeout = 30000; // Timeout pentru actualizări

void setup() {
    Serial.begin(115200);

    // Configurare IP static
    if (!WiFi.config(staticIP, gateway, subnet)) {
        Serial.println("Eroare la configurarea IP-ului static!");
    }

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConectat la WiFi!");

    // Configurare pini actuatoare
    for (int i = 0; i < 6; i++) {
        pinMode(actuators[i], OUTPUT);
        digitalWrite(actuators[i], HIGH);
    }

    // Configurare server
    server.on("/update", handleUpdate);
    server.begin();
    Serial.println("Server HTTP pornit!");
}

void loop() {
    server.handleClient();

    // Verificare zone cu date expirate
    for (int i = 0; i < 6; i++) {
        if (millis() - lastUpdate[i] > timeout) {
            Serial.printf("Zona %d: Date expirate. Resetare la 20°C\n", i + 1);
            temps[i] = 20.0;
        }
    }

    // Gestionare încălzire
    static unsigned long lastManageTime = 0;
    if (millis() - lastManageTime > 5000) {
        manageHeating();
        lastManageTime = millis();
    }

    // Sincronizare boiler
    static unsigned long lastSyncTime = 0;
    if (millis() - lastSyncTime > 5000) {
        checkBoilerState();
        lastSyncTime = millis();
    }
}

void handleUpdate() {
    int zone = server.arg("zone").toInt();
    float temp = server.arg("temp").toFloat();
    float setTemp = server.arg("setTemp").toFloat();

    if (zone >= 1 && zone <= 6) {
        temps[zone - 1] = temp;
        setTemps[zone - 1] = setTemp;
        lastUpdate[zone - 1] = millis();
        Serial.printf("Zona %d: Temp curenta %.1f C, Temp setata %.1f C\n", zone, temp, setTemp);
        server.send(200, "text/plain", "OK");
    } else {
        server.send(400, "text/plain", "Zona invalida");
    }
}

void manageHeating() {
    bool boilerOn = false;

    for (int i = 0; i < 6; i++) {
        if (temps[i] < setTemps[i] - 0.2) {
            digitalWrite(actuators[i], LOW);
            heatingRequest[i] = true;
            boilerOn = true;
        } else if (temps[i] > setTemps[i] + 0.2) {
            digitalWrite(actuators[i], HIGH);
            heatingRequest[i] = false;
        }
    }

    static bool lastBoilerState = false;
    if (boilerOn != lastBoilerState) {
        sendBoilerCommand(boilerOn);
        lastBoilerState = boilerOn;
    }
}

void sendBoilerCommand(bool turnOn) {
    if (WiFi.status() == WL_CONNECTED) {
        WiFiClient client;
        HTTPClient http;
        String url = "http://" + String(gasBoilerIP) + ":" + String(gasBoilerPort) + "/boiler?state=" + (turnOn ? "on" : "off");
        
        bool confirmed = false;
        int retryCount = 0;

        while (!confirmed && retryCount < 5) {
            if (http.begin(client, url)) {
                int httpResponseCode = http.GET();
                if (httpResponseCode == 200) {
                    String response = http.getString();
                    if ((turnOn && response == "Boiler ON") || (!turnOn && response == "Boiler OFF")) {
                        confirmed = true;
                    }
                }
                http.end();
            }
            retryCount++;
            delay(1000);
        }

        if (!confirmed) {
            Serial.println("Boilerul nu a confirmat starea. Retrimite comanda!");
        }
    }
}

void checkBoilerState() {
    if (WiFi.status() == WL_CONNECTED) {
        WiFiClient client;
        HTTPClient http;
        String url = "http://" + String(gasBoilerIP) + ":" + String(gasBoilerPort) + "/status";

        if (http.begin(client, url)) {
            int httpResponseCode = http.GET();
            if (httpResponseCode == 200) {
                String response = http.getString();
                bool expectedState = heatingRequest[0];
                if ((response == "Boiler ON" && !expectedState) || (response == "Boiler OFF" && expectedState)) {
                    sendBoilerCommand(expectedState);
                }
            }
            http.end();
        }
    }
}
