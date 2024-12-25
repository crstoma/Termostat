#include <WifiEspNow.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>

// Configurare senzor DHT
#define DHTPIN 4 // Pinul pentru DHT
#define DHTTYPE DHT22 // Tipul senzorului DHT
DHT dht(DHTPIN, DHTTYPE);

// Setări pentru OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// MAC Address of the recipient ESP32
static uint8_t PEER[]{0x1C, 0x69, 0x20, 0xCE, 0x80, 0xA9}; // Adresa MAC a ESP32

void printReceivedMessage(const uint8_t mac[WIFIESPNOW_ALEN], const uint8_t* buf, size_t count, void* arg) {
    Serial.printf("Message from %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    for (int i = 0; i < static_cast<int>(count); ++i) {
        Serial.print(static_cast<char>(buf[i]));
    }
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    dht.begin(); // Inițializează senzorul DHT

  // Inițializează I2C folosind pinii specificați
  Wire.begin(14, 12); // SDA = GPIO 14, SCL = GPIO 12
  
  // Inițializează OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;);
  }
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Adăugăm ESP32 ca peer
    WifiEspNow.begin();
    static uint8_t PEER[]{0x1C, 0x69, 0x20, 0xCE, 0x80, 0xA9}; // Adresa MAC a ESP32
    WifiEspNow.addPeer(PEER);
}

void loop() {
    // Citirea temperaturii și umidității
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // Verificare dacă citirea a avut succes
    if (!isnan(h) && !isnan(t)) {
        // Crearea mesajului JSON
        char msg[32];
        snprintf(msg, sizeof(msg), "{ \"temp\": %.1f }", t); // Formatează mesajul
        WifiEspNow.send(PEER, reinterpret_cast<const uint8_t*>(msg), strlen(msg)); // Trimitere mesaj
        Serial.printf("Trimis: %s\n", msg);
    } else {
        Serial.println("Eroare la citirea senzorului DHT!");
    }

    delay(2000); // Întârziere de 2 secunde între trimiteri
    DisplayDataRead();
    delay(2000); // Trimite la fiecare 2 secunde
}

void DisplayDataRead() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

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

  display.display();  // Actualizăm ecranul
}
