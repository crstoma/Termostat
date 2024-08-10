Materiale necesare
4 x ESP32
4 x Senzor de temperatură și umiditate (ex: DHT11 sau DHT22)
1 x Placă Arduino (ex: Arduino Uno)
1 x Modul de rețea pentru Arduino (ex: Ethernet Shield sau modul WiFi)
4 x Releu
Fire de conexiune și breadboard (placă de test)
Sursă de alimentare

Configurarea rețelei
Asigură-te că atât ESP32 cât și Arduino sunt conectate la aceeași rețea WiFi.
Înlocuiește ssid și password în codul ESP32 cu informațiile rețelei tale.

Conectarea senzorului DHT la ESP32
Conectează pinul VCC al senzorului la 3.3V pe ESP32.
Conectează pinul GND al senzorului la GND pe ESP32.
Conectează pinul Data al senzorului la un pin digital pe ESP32 (ex: GPIO 4).

Configurarea Arduino
Conectarea releelor la Arduino
Conectează pinul VCC al fiecărui releu la 5V pe Arduino.
Conectează pinul GND al fiecărui releu la GND pe Arduino.
Conectează pinii de control ai fiecărui releu la pinii digitali pe Arduino (ex: 2, 3, 4, 5).

