#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

// Configurare WiFi
const char* ssid = "WiFi";
const char* password = "password";

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

// Configurare DHT
#define DHTPIN 19
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
float dhtTemp = 0.0;

// Configurare LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);

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

    // Inițializare server
    server.on("/update", handleUpdate);
    server.begin();
    Serial.println("Server HTTP pornit!");

    // Inițializare DHT
    dht.begin();

    // Inițializare LCD
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.print("Initializare...");
    delay(2000);
}

void loop() {
    server.handleClient();

    // Actualizare temperatură DHT
    dhtTemp = dht.readTemperature();

    // Verificare zone cu date expirate
    for (int i = 0; i < 6; i++) {
    if (millis() - lastUpdate[i] > timeout) {
        Serial.printf("Zona %d: Pierdere conexiune. Folosim temperatura DHT\n", i + 1);
        temps[i] = dhtTemp;
        heatingRequest[i] = false; // Asigură-te că actuatorul este oprit
    }
}

    // Gestionare încălzire
    static unsigned long lastManageTime = 0;
    if (millis() - lastManageTime > 3000) {
        manageHeating();
        lastManageTime = millis();
    }

    // Sincronizare boiler
    static unsigned long lastSyncTime = 0;
    if (millis() - lastSyncTime > 3000) {
        checkBoilerState();
        lastSyncTime = millis();
    }

    // Actualizare afișaj
    updateLCD();
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

        Serial.printf("Trimitere comandă boiler: %s\n", turnOn ? "ON" : "OFF");

        if (http.begin(client, url)) {
            int httpResponseCode = http.GET();
            if (httpResponseCode == 200) {
                String response = http.getString();
                Serial.println("Răspuns boiler: " + response);
            } else {
                Serial.printf("Eroare la trimiterea comenzii către boiler. Cod HTTP: %d\n", httpResponseCode);
            }
            http.end();
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
                bool boilerState = (response == "Boiler ON");
                bool expectedState = false;
                for (int i = 0; i < 6; i++) {
                    if (heatingRequest[i]) {
                        expectedState = true;
                        break;
                    }
                }

                if (boilerState != expectedState) {
                    Serial.println("Starea boilerului nu corespunde. Reaplicare comandă...");
                    sendBoilerCommand(expectedState);
                }
            } else {
                Serial.printf("Eroare la obținerea stării boilerului. Cod HTTP: %d\n", httpResponseCode);
            }
            http.end();
        }
    }
}


void updateLCD() {
    static unsigned long lastLCDUpdate = 0; // Nume diferit pentru claritate
    unsigned long currentMillis = millis();

    // Actualizare la fiecare 1000 ms
    if (currentMillis - lastLCDUpdate >= 1000) {
        lastLCDUpdate = currentMillis;

        // Rând 1: Starea releelor 1-3
        lcd.setCursor(0, 0);
        for (int i = 0; i < 3; i++) {
            lcd.print("Z");
            lcd.print(i + 1);
            lcd.print(heatingRequest[i] ? "=ON " : "=OFF ");
        }

        // Rând 2: Starea releelor 4-6
        lcd.setCursor(0, 1);
        for (int i = 3; i < 6; i++) {
            lcd.print("Z");
            lcd.print(i + 1);
            lcd.print(heatingRequest[i] ? "=ON " : "=OFF ");
        }

        // Rând 3: Starea boilerului (CT) și sincronizarea (CON)
        lcd.setCursor(0, 2);
        lcd.print("CT=");
        lcd.print(heatingRequest[0] ? "ON " : "OFF");
        lcd.setCursor(11, 2); // 4 spații între CT și CON
        lcd.print("CON=");
        
        bool boilerSynced = true;
        // Verifică dacă toate zonele care solicită încălzire sunt sincronizate cu boilerul
        for (int i = 0; i < 6; i++) {
            if (heatingRequest[i] && (millis() - lastUpdate[i] > timeout)) {
                boilerSynced = false;
                break;
            }
        }

        lcd.print(boilerSynced ? "ON " : "OFF");

        // Rând 4: Temperatura DHT
        lcd.setCursor(0, 3);
        lcd.print("DHT=");
        lcd.print(dhtTemp, 1); // Afișează temperatura DHT
        lcd.print("C   "); // Adaugă spații pentru a acoperi caractere vechi
    }
}
