#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
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

// Butoane
#define BUTTON_UP 0       // Buton creștere
#define BUTTON_DOWN 5     // Buton scădere
#define BUTTON_SELECT 13  // Buton selectare
#define BUTTON_MENU 2     // Buton schimbare meniu (D4)

// Variabile pentru meniu
bool inMenu = false;
int selectedMenu = 0; // 0 = meniul pentru temperatură, 1 = meniul pentru ceas

// Variabile pentru temperatura setată
float setTemperature = 21.0;
float minTemp = 17.0;
bool inSetTempMode = false;  // Modul de setare a temperaturii

// Variabile pentru ora curentă
int currentHour = 0;
int currentMinute = 0;
int currentSecond = 0;
bool inSetTimeMode = false;
bool settingHours = true; // Inițial setăm orele

// Interval de actualizare
unsigned long previousMillis = 0;
const long interval = 1000; // 1 secundă

// Variabile pentru gestionarea butoanelor cu millis()
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200;  // 200 ms pentru debouncing
unsigned long lastPressTimeUp = 0;
unsigned long lastPressTimeDown = 0;
unsigned long pressInterval = 500;  // 500 ms pentru apăsare lungă

// Variabile pentru actualizarea timpului folosind millis()
unsigned long previousSecondMillis = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Inițializează I2C folosind pinii specificați
  Wire.begin(14, 12); // SDA = GPIO 14, SCL = GPIO 12
  
  // Inițializează OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;);
  }

  // Configurare pini butoane
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  pinMode(BUTTON_MENU, INPUT_PULLUP);

  // Afișăm datele senzorului la început
  DisplayDataRead();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Actualizare timp curent
  if (currentMillis - previousSecondMillis >= 1000) { // Verificăm dacă a trecut o secundă
    previousSecondMillis = currentMillis;
    currentSecond++;
    if (currentSecond >= 60) {
      currentSecond = 0;
      currentMinute++;
      if (currentMinute >= 60) {
        currentMinute = 0;
        currentHour++;
        if (currentHour >= 24) {
          currentHour = 0;
        }
      }
    }
  }

  // Afișăm datele dacă nu se apasă niciun buton și nu suntem în niciun meniu
  if (!inMenu && !inSetTempMode && !inSetTimeMode) {
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      DisplayDataRead();
    }
  }

  // Verificăm butonul de meniu pentru a intra în meniuri
  if (digitalRead(BUTTON_MENU) == LOW && (currentMillis - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = currentMillis;
    inMenu = !inMenu; // Comutăm între meniuri și afișare normală

    if (inMenu) {
      displayMainMenu();
    } else {
      DisplayDataRead(); // Revenim la afișarea senzorului când ieșim din meniu
    }
  }

  // Gestionăm meniul principal dacă suntem în el
  if (inMenu) {
    handleMainMenu();
  }

  // Gestionăm setarea temperaturii sau orei, în funcție de selecția din meniu
  if (inSetTempMode) {
    SetareTemp();
  } else if (inSetTimeMode) {
    setTime();
  }
}

// Afișare date temperatură și umiditate
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

  // Afișăm ora curentă
  display.setTextSize(1);
  display.setCursor(0, 56); // Setează poziția mai jos pe ecran pentru afișarea orei
  display.print("Ora: ");
  if (currentHour < 10) display.print("0"); // Afișăm ora cu două cifre
  display.print(currentHour);
  display.print(":");
  if (currentMinute < 10) display.print("0"); // Afișăm minutele cu două cifre
  display.print(currentMinute);
  display.print(":");
  if (currentSecond < 10) display.print("0"); // Afișăm secundele cu două cifre
  display.print(currentSecond);

  display.display();  // Actualizăm ecranul
}

// Afișare meniu principal cu selecție
void displayMainMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Main Menu:");

  // Indicator pentru meniu selectat
  display.setCursor(0, 16);
  if (selectedMenu == 0) display.print(" > ");
  display.println("Set Temp");

  display.setCursor(0, 32);
  if (selectedMenu == 1) display.print(" > ");
  display.println("Set Clock");

  display.display();
}

// Gestionarea meniului principal
void handleMainMenu() {
  unsigned long currentMillis = millis();

  // Buton UP pentru schimbare selecție meniu
  if (digitalRead(BUTTON_UP) == LOW && currentMillis - lastPressTimeUp > debounceDelay) {
    lastPressTimeUp = currentMillis;
    selectedMenu = (selectedMenu + 1) % 2; // Comutăm între 0 și 1
    displayMainMenu();
  }

  // Buton SELECT pentru a intra în meniul selectat
  if (digitalRead(BUTTON_SELECT) == LOW && currentMillis - lastDebounceTime > debounceDelay) {
    lastDebounceTime = currentMillis;
    inMenu = false; // Ieșim din meniul principal

    if (selectedMenu == 0) {
      inSetTempMode = true; // Intrăm în modul de setare a temperaturii
    } else if (selectedMenu == 1) {
      inSetTimeMode = true; // Intrăm în modul de setare a orei
    }
  }
}

// Funcție pentru setarea temperaturii cu butoanele UP și DOWN
void SetareTemp() {
  unsigned long currentMillis = millis();

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Set Temp:");
  display.setCursor(0, 40);
  display.print(setTemperature);
  display.print(" *C");
  display.display();

  // Apăsare lungă pe butonul UP (crește temperatura)
  if (digitalRead(BUTTON_UP) == LOW && currentMillis - lastPressTimeUp > pressInterval) {
    lastPressTimeUp = currentMillis;
    setTemperature += 0.5;
  }

  // Apăsare lungă pe butonul DOWN (scade temperatura)
  if (digitalRead(BUTTON_DOWN) == LOW && currentMillis - lastPressTimeDown > pressInterval) {
    lastPressTimeDown = currentMillis;
    if (setTemperature > minTemp) {
      setTemperature -= 0.5;
    }
  }

  // Buton SELECT pentru a salva temperatura și a ieși din modul de setare
  if (digitalRead(BUTTON_SELECT) == LOW && currentMillis - lastDebounceTime > debounceDelay) {
    lastDebounceTime = currentMillis;
    inSetTempMode = false; // Ieșim din modul de setare a temperaturii
  }
}

// Funcție pentru setarea orei
void setTime() {
  unsigned long currentMillis = millis();

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Set Time:");

  // Afișăm ora curentă pentru setare
  display.setCursor(0, 40);
  if (settingHours) {
    display.print("Hour: ");
    display.print(currentHour);
  } else {
    display.print("Minute: ");
    display.print(currentMinute);
  }
  display.display();

  // Buton UP pentru a crește ora/minutul
  if (digitalRead(BUTTON_UP) == LOW && currentMillis - lastPressTimeUp > pressInterval) {
    lastPressTimeUp = currentMillis;
    if (settingHours) {
      currentHour = (currentHour + 1) % 24; // Crește ora (0-23)
    } else {
      currentMinute = (currentMinute + 1) % 60; // Crește minutul (0-59)
    }
  }

  // Buton DOWN pentru a scădea ora/minutul
  if (digitalRead(BUTTON_DOWN) == LOW && currentMillis - lastPressTimeDown > pressInterval) {
    lastPressTimeDown = currentMillis;
    if (settingHours) {
      currentHour = (currentHour + 23) % 24; // Scade ora (0-23)
    } else {
      currentMinute = (currentMinute + 59) % 60; // Scade minutul (0-59)
    }
  }

  // Buton SELECT pentru a schimba între setarea orei și minutei
  if (digitalRead(BUTTON_SELECT) == LOW && currentMillis - lastDebounceTime > debounceDelay) {
    lastDebounceTime = currentMillis;
    settingHours = !settingHours; // Comutăm între setarea orei și minutei
  }

  // Buton MENU pentru a salva ora și a ieși din modul de setare
  if (digitalRead(BUTTON_MENU) == LOW && currentMillis - lastDebounceTime > debounceDelay) {
    lastDebounceTime = currentMillis;
    inSetTimeMode = false; // Ieșim din modul de setare a timpului
  }
}
