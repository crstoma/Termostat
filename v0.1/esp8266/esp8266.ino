#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Setări pentru rețeaua WiFi
const char* ssid = "EpicMe";      // Înlocuiește cu numele rețelei tale Wi-Fi
const char* password = "cristofer27";  // Înlocuiește cu parola rețelei tale Wi-Fi

// Adresa IP a Arduino-ului în rețea
const char* serverIP = "192.168.0.150";  // Adresa IP a Arduino-ului (IP-ul static setat pe Arduino)

// Pinul la care este conectat senzorul DHT22
#define DHTPIN 4        // Pinul pe care ai conectat senzorul DHT22 la ESP8266
#define DHTTYPE DHT22   // Tipul de senzor (DHT22)

// Dimensiuni OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Inițializează senzorul DHT
DHT dht(DHTPIN, DHTTYPE);

// Definire OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  // Inițializează conexiunea serială
  Serial.begin(115200);

  // Inițializează senzorul DHT
  dht.begin();

  // Conectează ESP8266 la rețeaua WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Afișează adresa IP alocată pentru ESP8266
  Serial.print("ESP8266 IP Address: ");
  Serial.println(WiFi.localIP());

  // Inițializează OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Adresa I2C a OLED-ului este de obicei 0x3C
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Stop executia daca nu gaseste OLED-ul
  }

  display.clearDisplay();
  display.setTextSize(1);             // Setează dimensiunea textului
  display.setTextColor(SSD1306_WHITE);  // Setează culoarea textului
  display.setCursor(0, 0);
  display.print("ESP8266 Initialized");
  display.display();                  // Actualizează ecranul
  delay(2000);
}

void loop() {
  // Citește temperatura de la senzorul DHT22
  float temperature = dht.readTemperature();
  
  // Verifică dacă temperatura este validă
  if (isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Afișează temperatura citită în consola serială
  Serial.print("Temperature: ");
  Serial.println(temperature);

  // Afișează temperatura pe OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Temperature: ");
  display.print(temperature);
  display.println(" C");
  display.display();

  // Trimite datele către Arduino (controlul releului 1)
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Creează un obiect WiFiClient
    WiFiClient client;

    // Creează URL-ul pentru cererea HTTP către Arduino
    String serverPath = "http://" + String(serverIP) + "/setTemp=" + String(temperature) + "&releu=1";

    // Debug: Afișează URL-ul
    Serial.println("Requesting URL: " + serverPath);

    // Trimite cererea GET către server (Arduino)
    http.begin(client, serverPath);  // Modificarea aici
    int httpResponseCode = http.GET();

    // Verifică dacă cererea a avut succes
    if (httpResponseCode > 0) {
      String response = http.getString();  // Răspunsul de la server
      Serial.print("Response: ");
      Serial.println(response);
    } else {
      Serial.print("Error in HTTP request: ");
      Serial.println(httpResponseCode);
    }

    // Închide conexiunea
    http.end();
  } else {
    Serial.println("WiFi disconnected!");
  }

  // Așteaptă 10 secunde înainte de a citi din nou temperatura
  delay(10000);
}
