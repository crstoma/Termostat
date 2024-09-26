#include <SPI.h>
#include <Ethernet.h>

// Configurarea adresei MAC și IP-ului
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // Setează o adresă MAC unică
IPAddress ip(192, 168, 0, 150); // Setează adresa IP statică

EthernetServer server(80); // Server pe portul 80

void setup() {
  // Inițializează Ethernet
  Ethernet.begin(mac, ip);
  server.begin(); // Începe serverul
  Serial.begin(9600);
  Serial.print("Server is at ");
  Serial.println(Ethernet.localIP());
  
  // Configurează pinul pentru releu (ex. pinul 2)
  pinMode(2, OUTPUT); // Releul 1 pe pinul 2
  digitalWrite(2, LOW); // Asigură-te că releul este oprit la început
}

void loop() {
  EthernetClient client = server.available(); // Așteaptă un client
  if (client) {
    String currentLine = ""; // Variabilă pentru linia curentă
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c); // Afișează în monitorul serial
        if (c == '\n') {
          // Verifică dacă linia curentă este goală
          if (currentLine.length() == 0) {
            // Trimite un răspuns HTTP
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.println("<html><body><h1>Request Received!</h1></body></html>");

            // Aici poți procesa cererea, extragerea datelor
            if (currentLine.startsWith("GET /setTemp=")) {
              int tempStart = currentLine.indexOf('=') + 1;
              int tempEnd = currentLine.indexOf('&', tempStart);
              String tempValue = currentLine.substring(tempStart, tempEnd);
              Serial.print("Received temperature: ");
              Serial.println(tempValue);
              
              // Verifică dacă trebuie să activezi releul
              if (currentLine.indexOf("releu=1") != -1) {
                Serial.println("Activating Relay 1");
                digitalWrite(2, HIGH); // Activează releul
              } else {
                digitalWrite(2, LOW); // Dezactivează releul
              }
            }
            break; // Închide conexiunea
          } else {
            currentLine = ""; // Resetează linia curentă
          }
        } else if (c != '\r') {
          currentLine += c; // Adaugă caracter la linia curentă
        }
      }
    }
    client.stop(); // Închide conexiunea
    Serial.println("Client disconnected");
  }
}
