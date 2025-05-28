#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "hey";
const char* password = "159357123";

ESP8266WebServer server(80);

String dataToSend = "ESP initial data";
String receivedFromPython = "";

int counter = 0;
unsigned long lastPrintTime = 0;

void handleReceive() {
  server.send(200, "text/plain", dataToSend);
}

void handleSend() {
  if (server.hasArg("value")) {
    receivedFromPython = server.arg("value");
    Serial.println("Received from Python (in handler): " + receivedFromPython);
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("ESP IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/receive", handleReceive);
  server.on("/send", handleSend);

  server.begin();
}

void loop() {
  server.handleClient();

  // Update the variable that will be sent back to Python
  dataToSend = "ESP counter: " + String(counter++);
  
  // Optional: print the received value every second
  if (millis() - lastPrintTime >= 1000) {
    Serial.println("Loop-accessed received value: " + receivedFromPython);
    lastPrintTime = millis();
  }

  delay(10); // Small delay to keep it responsive
}
