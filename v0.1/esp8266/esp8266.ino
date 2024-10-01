// Created by EpicMe

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

#define BATT_LEVEL A0
const float BATT_LEVEL_LOW = 3.5;
//Button
#define BUTTON_PIN 5  // GPIO5 (D1 pe NodeMCU) pentru buton
// Configurare rețea
const char* ssid = "WiFi";
const char* password = "passwword";
const char* serverIP = "192.168.0.150"; // IP-ul Arduino-ului

WiFiClient client;

void setup() {
  // put your setup code here, to run once:

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
  display.ssd1306_command(5);  // Setează o valoare de contrast mai mică (exemplu: 50)

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
  // put your main code here, to run repeatedly:
  //Trimite date catre arduino
  SendData();

  //Citire Button si afisare pe ecran oled
  if(digitalRead(BUTTON_PIN) == LOW){
    delay(50);
    display.ssd1306_command(SSD1306_DISPLAYON);
    DisplayDataRead();
    delay(10000);
  }else{
    display.ssd1306_command(SSD1306_DISPLAYOFF);
  }


  delay(5000);
}

void SendData() {

  float temperature = dht.readTemperature();
  if (client.connect(serverIP, 81)) {
    client.print(String("Temp=") + temperature);
    client.stop();
  }
}


void DisplayDataRead(){

// Citește temperatura și umiditatea
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float battLevelNow = analogRead(BATT_LEVEL) / 209.66; // assumes external 180K resistor
  Serial.print("batt=");
  Serial.println(battLevelNow);

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

  display.setCursor(0, 56);
  display.setTextSize(1);
  display.print(F("Battery: "));
  display.print(battLevelNow, 2);
  display.print(F(" V"));

  display.display();

}

void BatteryMonitor(){

  float battLevelNow = analogRead(BATT_LEVEL) / 209.66; // assumes external 180K resistor
  Serial.print("batt=");
  Serial.println(battLevelNow);

}
