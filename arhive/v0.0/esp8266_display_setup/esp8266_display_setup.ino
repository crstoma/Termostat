#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// Setări pentru OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Setări pentru senzorul DHT
#define DHTPIN 4      // GPIO 4 pentru senzorul DHT (poți folosi orice alt pin disponibil)
#define DHTTYPE DHT22 // AM2302 este un DHT22
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // Inițializează I2C folosind pinii specificați
  Wire.begin(14, 12); // SDA = GPIO 14, SCL = GPIO 12
  
  // Inițializează OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for(;;); // Nu a reușit să inițializeze OLED-ul
  }
  display.clearDisplay();
  
  // Inițializează senzorul DHT
  dht.begin();

  // Inițializare serial monitor pentru debugging
  Serial.begin(115200);
}

void loop() {
  // Citește temperatura și umiditatea
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Verifică dacă citirea este corectă
  if (isnan(h) || isnan(t)) {
    Serial.println("Eroare la citirea senzorului DHT!");
    return;
  }

  // Afișează pe OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  
  display.println("Temp: " + String(t) + " C");
  display.println("Umid: " + String(h) + " %");
  
  display.display();
  
  // Așteaptă 2 secunde înainte de următoarea citire
  delay(2000);
}
