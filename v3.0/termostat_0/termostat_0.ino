#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>

// Configurare WiFi
const char* ssid = "wifi";
const char* password = "password";

// Configurare DHT
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Configurare OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Configurare pini I2C
#define SDA_PIN 14
#define SCL_PIN 12

// Configurare butoane
#define BTN_UP 0
#define BTN_DOWN 5
#define BTN_SELECT 13

// Configurare centrale de comandă
const char* boilerIP = "192.168.0.100"; // IP-ul centralei de comandă
const int boilerPort = 80;
const int thermostatID = 1; // ID unic al acestui termostat

// Variabile pentru temperatură
float targetTemp = 22.0;
float hyst = 0.2;
bool heating = false;

// Variabile pentru gestionarea butoanelor
unsigned long lastButtonCheck = 0; // Momentul ultimei verificări
const unsigned long buttonCheckInterval = 200; // Interval mai lung pentru debouncing
bool btnUpPressed = false;
bool btnDownPressed = false;
bool btnSelectPressed = false;

// Stare meniu
bool inMenu = false;

void setup() {
  Serial.begin(115200);

  // Configurare WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectat la WiFi!");

  // Inițializare DHT
  dht.begin();

  // Inițializare I2C și OLED
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Eroare la inițializarea OLED!");
    for (;;);
  }

  // Inițializare butoane
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);

  // Mesaj inițial pe OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Termostat pornit!");
  display.display();
  delay(2000);
}

void loop() {
  checkButtons();
  // Verifică periodic starea centralei
  checkBoilerStatus();
  if (inMenu) {
    updateDisplayMenu();
  } else {
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    if (isnan(temp) || isnan(hum)) {
      Serial.println("Eroare la citirea senzorului DHT22!");
      return;
    }

    // Control histerezis
    if (temp < (targetTemp - hyst) && !heating) {
      heating = true;
      sendSignalToBoiler(true);
    } else if (temp > (targetTemp + hyst) && heating) {
      heating = false;
      sendSignalToBoiler(false);
    }

    // Actualizare afișaj OLED
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.printf("T: %.1f C\n", temp);
    display.printf("U: %.1f %%\n", hum);
    display.printf("SET:%.1f C\n", targetTemp);
    display.setTextSize(1);
    display.printf("Centrala: %s\n", heating ? "Pornit" : "Oprit");
    display.display();

    delay(2000);
  }
}

void checkButtons() {
  if (millis() - lastButtonCheck >= buttonCheckInterval) {
    lastButtonCheck = millis();

    // Citire stare butoane
    bool btnUpState = digitalRead(BTN_UP) == LOW;
    bool btnDownState = digitalRead(BTN_DOWN) == LOW;
    bool btnSelectState = digitalRead(BTN_SELECT) == LOW;

    // Gestionare butoane
    if (btnUpState && !btnUpPressed) {
      btnUpPressed = true;
      if (inMenu) {
        targetTemp += 0.5;
        if (targetTemp > 30.0) targetTemp = 30.0;
      }
    } else if (!btnUpState) {
      btnUpPressed = false;
    }

    if (btnDownState && !btnDownPressed) {
      btnDownPressed = true;
      if (inMenu) {
        targetTemp -= 0.5;
        if (targetTemp < 19.0) targetTemp = 19.0;
      }
    } else if (!btnDownState) {
      btnDownPressed = false;
    }

    if (btnSelectState && !btnSelectPressed) {
      btnSelectPressed = true;
      inMenu = !inMenu;
    } else if (!btnSelectState) {
      btnSelectPressed = false;
    }
  }
}

void updateDisplayMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  if (inMenu) {
    display.setCursor(0, 0);
    display.println("SETARE TEMPERATURA");
    display.setCursor(0, 20);
    display.setTextSize(2);
    display.printf("SET:%.1f C\n", targetTemp);
    display.setTextSize(1);
    display.println("UP/DOWN: Modifica");
    display.println("SELECT: Salveaza");
  }

  display.display();
}

void sendSignalToBoiler(bool state) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    String url = String("http://") + boilerIP + ":" + boilerPort +
                 "/control?id=" + thermostatID +
                 "&state=" + (state ? "on" : "off");

    if (http.begin(client, url)) {
      int httpCode = http.GET();
      if (httpCode > 0) {
        Serial.println("Semnal trimis către centrală!");
      } else {
        Serial.println("Eroare la trimiterea semnalului!");
      }
      http.end();
    } else {
      Serial.println("Nu s-a putut inițializa conexiunea HTTP!");
    }
  } else {
    Serial.println("Nu există conexiune WiFi!");
  }
}

void checkBoilerStatus() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    String url = String("http://") + boilerIP + "/status";  // Ruta care returnează statusul centralei

    if (http.begin(client, url)) {
      int httpCode = http.GET();
      if (httpCode > 0) {
        String response = http.getString();
        if (response.indexOf("pornita") >= 0) {
          heating = true;
        } else {
          heating = false;
        }
        Serial.println("Status centrala: " + response);
      } else {
        Serial.println("Eroare la cererea statusului centralei");
      }
      http.end();
    } else {
      Serial.println("Nu s-a putut inițializa conexiunea HTTP!");
    }
  } else {
    Serial.println("Nu există conexiune WiFi!");
  }
}
