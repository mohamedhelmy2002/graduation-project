// ########################## DEFINES ##########################
#define SERIAL_BAUD         115200      // Baud rate for Serial Monitor
#define START_FRAME         0xABCD      // Start frame for reliable serial communication
#define TIME_SEND           100         // [ms] Sending time interval
#define SPEED_MAX_TEST      300         // Maximum speed for testing
#define SPEED_STEP          20          // Speed step
// #define DEBUG_RX                      // Debug received data

#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ########################## GLOBALS ##########################
unsigned long iTimeSend = 0;
uint8_t idx = 0;
uint16_t bufStartFrame;
byte *p;
byte incomingByte;
byte incomingBytePrev;

unsigned long deltaValid = 0;               // Time between valid packets
unsigned long lastValidReceiveTime = 0;     // Last time a valid packet was received

typedef struct {
   uint16_t start;
   int16_t  steer;
   int16_t  speed;
   uint16_t checksum;
} SerialCommand;
SerialCommand Command;

typedef struct {
   uint16_t start;
   int16_t  cmd1;
   int16_t  cmd2;
   int16_t  speedR_meas;
   int16_t  speedL_meas;
   int16_t  batVoltage;
   int16_t  boardTemp;
   uint16_t cmdLed;
   uint16_t checksum;
} SerialFeedback;
SerialFeedback Feedback;
SerialFeedback NewFeedback;

const char* ssid = "hey";
const char* password = "159357123";
ESP8266WebServer server(80);

String dataToSend = "ESP initial data";
String receivedFromPython = "";
int counter = 0;
unsigned long lastPrintTime = 0;

int speed = 0;
int steering = 0;

// ########################## SEND ##########################
void Send(int16_t uSteer, int16_t uSpeed)
{
  Command.start    = (uint16_t)START_FRAME;
  Command.steer    = (int16_t)uSteer;
  Command.speed    = (int16_t)uSpeed;
  Command.checksum = (uint16_t)(Command.start ^ Command.steer ^ Command.speed);
  Serial.write((uint8_t *) &Command, sizeof(Command)); 
}

// ########################## RECEIVE ##########################
void Receive()
{
    static unsigned long lastCallTime = 0;
    unsigned long currentReceiveTime = millis();
    unsigned long deltaCall = currentReceiveTime - lastCallTime;
    lastCallTime = currentReceiveTime;

    if (Serial.available()) {
        incomingByte = Serial.read();
        bufStartFrame = ((uint16_t)(incomingByte) << 8) | incomingBytePrev;
    } else {
        return;
    }

    #ifdef DEBUG_RX
        Serial.print(incomingByte);
        return;
    #endif

    if (bufStartFrame == START_FRAME) {
        p = (byte *)&NewFeedback;
        *p++ = incomingBytePrev;
        *p++ = incomingByte;
        idx = 2;
    } else if (idx >= 2 && idx < sizeof(SerialFeedback)) {
        *p++ = incomingByte;
        idx++;
    }

    if (idx == sizeof(SerialFeedback)) {
        uint16_t checksum = (uint16_t)(NewFeedback.start ^ NewFeedback.cmd1 ^ NewFeedback.cmd2 ^
                              NewFeedback.speedR_meas ^ NewFeedback.speedL_meas ^
                              NewFeedback.batVoltage ^ NewFeedback.boardTemp ^ 
                              NewFeedback.cmdLed);

        if (NewFeedback.start == START_FRAME && checksum == NewFeedback.checksum) {
            memcpy(&Feedback, &NewFeedback, sizeof(SerialFeedback));
            deltaValid = currentReceiveTime - lastValidReceiveTime;
            lastValidReceiveTime = currentReceiveTime;
        } else {
            Serial.println("Non-valid data skipped");
        }
        idx = 0;
    }

    incomingBytePrev = incomingByte;
}

// ########################## COMMUNICATION ##########################
void handleReceive() {
  server.send(200, "text/plain", dataToSend);
}

void handleSend() {
  if (server.hasArg("value")) {
    receivedFromPython = server.arg("value");
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

// ########################## SETUP ##########################
void setup() 
{
  Serial.begin(SERIAL_BAUD);
  Serial.println("Hoverboard Serial v1.0");

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

  pinMode(LED_BUILTIN, OUTPUT);
}

// ########################## LOOP ##########################
void loop()
{
  unsigned long timeNow = millis();

  // Receive feedback from serial
  Receive();

  // Web server and LED blink
  server.handleClient();
  digitalWrite(LED_BUILTIN, (timeNow % 2000) < 1000);

  // Update string to be sent to Python
  dataToSend = "st: " + String(Feedback.cmd1) +
               " sp: " + String(Feedback.cmd2) +
               " sp_r: " + String(Feedback.speedR_meas) +
               " sp_l: " + String(Feedback.speedL_meas) +
               " bat_V: " + String(Feedback.batVoltage) +
               " temp: " + String(Feedback.boardTemp) +
               " d_t: " + String(deltaValid);

  // Parse command from Python every 100 ms
  if (millis() - lastPrintTime >= 100) {
    int separatorIndex = receivedFromPython.indexOf(',');
  if (separatorIndex != -1) {
    String steeringStr = receivedFromPython.substring(0, separatorIndex);
    String speedStr = receivedFromPython.substring(separatorIndex + 1);
    steering = steeringStr.toInt();
    speed = speedStr.toInt();
    //Serial.print("st : "); Serial.print(steering);
    //Serial.print(" sp : "); Serial.println(speed);
}


    lastPrintTime = millis();
  }

  // Send to motor at regular intervals
  if (iTimeSend > timeNow) return;
  iTimeSend = timeNow + TIME_SEND;
  Send(steering, speed);

  delay(10);
}
