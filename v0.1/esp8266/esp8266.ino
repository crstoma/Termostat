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
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);


// Configurare rețea
const char* ssid = "Wifi";
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
  
  // Conectare la rețea
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectat la retea!");
}

void loop() {
  float temperature = dht.readTemperature();

  // Afișare pe OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Temp: ");
  display.print(temperature);
  display.display();
  Serial.println(temperature);


  // Trimitere temperatură către Arduino prin rețea
  if (client.connect(serverIP, 81)) {
    client.print(String("Temp=") + temperature);
    client.stop();
  }
  
  delay(5000); // Repetă la fiecare 5 secunde
}
