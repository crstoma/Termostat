// Created by EpicMe
// updates
// Battery monitor
// Button for Display

#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>

// Setează pinul și tipul senzorului DHT
// Setări pentru OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

//Button
#define BUTTON_PIN 5  // GPIO5 (D1 pe NodeMCU) pentru buton

bool displayOn = false;  // Inițial, afișajul este oprit
unsigned long previousMillis = 0;  // Timpul la care afișajul a fost activat
const long interval = 10000;  // Interval de 10 secunde

// Pinul pentru măsurarea tensiunii bateriei
#define BATTERY_PIN A0  // Pinul analogic pentru măsurarea tensiunii bateriei
float batteryVoltage = 0.0;  // Variabilă pentru tensiunea bateriei
const float BATTERY_LOW_VOLTAGE = 3.3;  // Pragul pentru tensiune scăzută

// Configurare rețea
const char* ssid = "WiFi";
const char* password = "Password";
const char* serverIP = "192.168.0.150"; // IP-ul Arduino-ului

WiFiClient client;

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // Inițializează I2C folosind pinii specificați
  Wire.begin(14, 12); // SDA = GPIO 14, SCL = GPIO 12

  // Inițializează OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for(;;); // Nu a reușit să inițializeze OLED-ul
  }
  display.clearDisplay();

 // Setează contrastul (luminozitatea) OLED-ului
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(50);  // Setează o valoare de contrast mai mică (exemplu: 50)

  //Button
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Setează pinul butonului cu rezistență pull-up internă

  // Conectare la rețea
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectat la rețea!");
}

void loop() {
  unsigned long currentMillis = millis();  // Timpul curent

  // Citește temperatura și umiditatea
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Citește tensiunea bateriei
  batteryVoltage = analogRead(BATTERY_PIN) * (3.7 / 1023.0);  // Conversie în tensiune (maxim 3.7V)

  // Verifică dacă butonul este apăsat (starea LOW)
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50);  // Anti-debounce
    if (digitalRead(BUTTON_PIN) == LOW) {  // Verifică din nou dacă butonul este încă apăsat
      if (!displayOn) {
        // Trezește afișajul
        display.ssd1306_command(SSD1306_DISPLAYON);
        displayOn = true;
        previousMillis = currentMillis;  // Salvează timpul la care a fost aprins afișajul

        // Afișare pe OLED
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(40, 0);
        display.println(F("Welcome"));

        display.setTextSize(2);
        display.setCursor(0, 16);
        display.print(F("T:"));
        display.print(temperature);
        display.print(F(" C"));

        display.setCursor(0, 40);
        display.print(F("H:"));
        display.print(humidity);
        display.print(F(" %"));

        // Afișează mesaj de baterie slabă, dacă este cazul
        if (batteryVoltage < BATTERY_LOW_VOLTAGE) {
          display.setCursor(0, 56);
          display.setTextSize(1);
          display.print(F("Battery Low!"));
        }
        
        display.display();
      }
    }
  }

  // Verifică dacă afișajul este aprins și dacă au trecut 10 secunde
  if (displayOn && (currentMillis - previousMillis >= interval)) {
    display.ssd1306_command(SSD1306_DISPLAYOFF);  // Oprește afișajul
    displayOn = false;
  }

  // Trimitere temperatură către Arduino prin rețea
  if (client.connect(serverIP, 81)) {
    client.print(String("Temp=") + temperature);
    client.stop();
  }

  delay(5000); // Repetă la fiecare 5 secunde
}
