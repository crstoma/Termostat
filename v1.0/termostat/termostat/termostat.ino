#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>

// Configurare DHT22
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Setări pentru OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Configurare WiFi
const char* ssid = "WIRELESS";
const char* password = "PASSWORD";
const char* serverIP = "192.168.0.100"; // IP-ul centralei
const int serverPort = 8080;           // Portul centralei

// Configurare IP static
IPAddress staticIP(192, 168, 0, 101);  // IP-ul termostatului
IPAddress gateway(192, 168, 0, 1);     // Gateway
IPAddress subnet(255, 255, 255, 0);    // Masca de rețea

// Configurare butoane
#define BTN_INC 0   // Buton pentru creșterea temperaturii
#define BTN_DEC 5  // Buton pentru scăderea temperaturii
#define BTN_RESET 13 // Buton pentru resetarea temperaturii
float tempSet = 20.0; // Temperatura setată
String comStatus = "N/A"; // Status comunicare
String zona = "Living";   // Zona curentă

void setup() {
  Serial.begin(115200);

  // Configurare IP static
  if (!WiFi.config(staticIP, gateway, subnet)) {
    Serial.println("Eroare la configurarea IP-ului static!");
  }

  // Conectare la WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectat la WiFi!");
  Serial.print("IP al termostatului: ");
  Serial.println(WiFi.localIP());

  // Configurare DHT22
  dht.begin();

  // Inițializează I2C folosind pinii specificați
  Wire.begin(14, 12); // SDA = GPIO 14, SCL = GPIO 12
  
  // Inițializează OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;);
  }
  display.clearDisplay();
  display.display();

  // Configurare butoane
  pinMode(BTN_INC, INPUT_PULLUP);
  pinMode(BTN_DEC, INPUT_PULLUP);
  pinMode(BTN_RESET, INPUT_PULLUP);
}

void loop() {
  // Verificare conexiune WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reîncercare conectare WiFi...");
    WiFi.reconnect();
    delay(5000);
    comStatus = "Eroare WiFi";
    return;
  }

  // Gestionare butoane
  handleButtons();

  // Citire temperatură
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Eroare la citirea senzorului DHT22!");
    comStatus = "Eroare DHT";
    return;
  }

  // Trimitere date la server
  WiFiClient client;
  HTTPClient http;

  String url = "http://" + String(serverIP) + ":" + String(serverPort) + "/update?zone=1&temp=" + String(temp) + "&setTemp=" + String(tempSet);
  if (http.begin(client, url)) {
    int responseCode = http.GET();
    if (responseCode > 0) {
      Serial.printf("Răspuns server: %d\n", responseCode);
      comStatus = "OK";
    } else {
      Serial.printf("Eroare la trimiterea datelor: %s\n", http.errorToString(responseCode).c_str());
      comStatus = "Eroare Server";
    }
    http.end();
  } else {
    comStatus = "Eroare HTTP";
  }

  // Actualizare display
  updateDisplay(temp);

  //delay(5000); // Interval actualizare
}

void handleButtons() {
  static unsigned long lastPressTime = 0;
  unsigned long currentTime = millis();

  if (currentTime - lastPressTime > 200) { // Debounce
    if (digitalRead(BTN_INC) == LOW) {
      tempSet += 0.5; // Crește temperatura
      Serial.println("Buton + apăsat. Temp setată: " + String(tempSet));
      lastPressTime = currentTime;
    } else if (digitalRead(BTN_DEC) == LOW) {
      tempSet -= 0.5; // Scade temperatura
      Serial.println("Buton - apăsat. Temp setată: " + String(tempSet));
      lastPressTime = currentTime;
    } else if (digitalRead(BTN_RESET) == LOW) {
      tempSet = 20.0; // Resetează temperatura
      Serial.println("Buton reset apăsat. Temp setată: " + String(tempSet));
      lastPressTime = currentTime;
    }
  }
}

void updateDisplay(float temp) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  // Afișează temperatura setată
  display.setCursor(0, 0);
  display.print("SET:");
  display.print(tempSet);
  display.println("C");

  // Afișează temperatura curentă
  display.setCursor(0, 16);
  display.print("TEMP:");
  display.print(temp);
  display.println("C");

  // Afișează statusul comunicării
  display.setCursor(0, 32);
  display.print("Com: ");
  display.print(comStatus);

  // Afișează zona
  display.setCursor(0, 48);
  display.print("  ");
  display.print(zona);

  // Finalizează afișajul
  display.display();
}
