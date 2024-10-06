#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>

// Configurare senzor DHT
#define DHTPIN 4 // Pinul pentru DHT (D2 pe NodeMCU)
#define DHTTYPE DHT22 // Tipul senzorului DHT
DHT dht(DHTPIN, DHTTYPE);

// Setări pentru OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Citire Baterie
#define BATT_LEVEL A0
const float BATT_LEVEL_LOW = 3.5;

//Butoane setare temp
#define BUTTON_UP 0 //D3
#define BUTTON_DOWN 5 //D1
#define BUTTON_SELECT 13 //D7

// Variabilă pentru temperatura setată
float setTemperature = 21.0; // Temperatura implicită
float minTemp = 17.0; //Temperatura minima
// Variabilă globală pentru modul de setare a temperaturii
bool inSetTempMode = false;

// Configurare Wi-Fi
const char* ssid = "wifi"; // SSID-ul rețelei Wi-Fi
const char* password = "password"; // Parola rețelei Wi-Fi
const char* serverIP = "192.168.0.150"; // IP-ul Arduino-ului

void setup() {

  Serial.begin(115200);
  dht.begin();

// Inițializează I2C folosind pinii specificați
  Wire.begin(14, 12); // SDA = GPIO 14, SCL = GPIO 12

  // Inițializează OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for(;;); // Nu a reușit să inițializeze OLED-ul
  }

 // Setează contrastul (luminozitatea) OLED-ului
  //display.ssd1306_command(SSD1306_DISPLAYOFF);
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(5);  // Setează o valoare de contrast mai mică (exemplu: 50)
  display.clearDisplay();

// Configurare pini butoane
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);

  // Conectare la Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectat la rețea!");

}

void loop() {

  // Dacă suntem în modul de setare a temperaturii, rămânem acolo
  if (inSetTempMode) {
    SetareTemp();
  } else {
    // Verificăm dacă este apăsat vreun buton
    if (digitalRead(BUTTON_UP) == LOW || digitalRead(BUTTON_DOWN) == LOW) {
      inSetTempMode = true;  // Intrăm în modul de setare a temperaturii
      SetareTemp();
    } else {
      DisplayDataRead();  // Afișează datele curente
      RelaySend();        // Trimite comanda către releu
    }
  }
  
    //delay(5000); // Așteaptă 10 secunde înainte de a citi din nou
    //ESP.deepSleep(60e6);

}

void DisplayDataRead(){
  //display.ssd1306_command(SSD1306_DISPLAYON); 
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

void SetareTemp() {
  static unsigned long lastPressTime = 0;
  unsigned long currentTime = millis();

  // Debounce simplu
  if (currentTime - lastPressTime < 300) {
    return;  // Nu acceptăm o nouă apăsare până la 300ms
  }

  // Creștere temperatură
  if (digitalRead(BUTTON_UP) == LOW) {
    lastPressTime = currentTime;
    setTemperature += 0.5; // Crește temperatura setată cu 0.5 grade
    displayTemperature();
  }

  // Scădere temperatură
  if (digitalRead(BUTTON_DOWN) == LOW) {
    lastPressTime = currentTime;
    if (setTemperature > minTemp) {  // Verificăm dacă temperatura este peste minim
      setTemperature -= 0.5;  // Scade temperatura setată cu 0.5 grade
    } else {
      Serial.println("Temperatura nu poate fi mai mica decat " + String(minTemp) + " grade.");
    }
    displayTemperature();
  }

  // Salvare și ieșire din modul de setare a temperaturii
  if (digitalRead(BUTTON_SELECT) == LOW) {
    lastPressTime = currentTime;
    Serial.println("Temperatura setată: " + String(setTemperature));
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.print("Save Temp:");
    display.setCursor(0, 40);
    display.print(setTemperature);
    display.print(" *C");
    display.display();
    delay(2000);  // Afișează temperatura setată timp de 2 secunde

    inSetTempMode = false;  // Ieșim din modul de setare și revenim la afișajul principal
  }
}
// Funcție pentru afișarea temperaturii pe ecran
void displayTemperature() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Set Temp:");
  display.setCursor(0, 40);
  display.print(setTemperature);
  display.print(" *C");
  display.display();
}

// Funcție care trimite comanda către releu pe baza temperaturii citite
void RelaySend() {
  float temperature = dht.readTemperature();

  // Verifică dacă citirea temperaturii este validă
  if (isnan(temperature)) {
    Serial.println("Eroare citire DHT");
    return; // Dacă nu poate citi temperatura, ieșim din funcție
  }

  // Compară temperatura citită cu temperatura setată
  if (temperature < setTemperature) {
    sendRelayCommand(1); // Releu ON (temperatura citită este mai mică decât cea setată)
  } else {
    sendRelayCommand(0); // Releu OFF (temperatura citită este mai mare sau egală cu cea setată)
  }
}


void sendRelayCommand(int state) {
  WiFiClient client;
  float temperature = dht.readTemperature();
  if (client.connect(serverIP, 81)) { // Conectare la server
    String command = "releu=" + String(state); // Formează comanda releului
    client.print(command); // Trimiterea comenzii
    client.stop(); // Închide conexiunea
    Serial.print("Comandă trimisă: ");
    Serial.println(command);
  } else {
    Serial.println("Conexiune eșuată la server.");
  }
}