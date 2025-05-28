#include <HardwareSerial.h>

#define HOVER_SERIAL_BAUD   115200    // Baud rate for Hoverboard
#define SERIAL_BAUD         115200    // Baud rate for debugging
#define START_FRAME         0xABCD    // Start frame for reliable communication
#define TIME_SEND           100       // Sending time interval
#define SPEED_MAX_TEST      300       // Maximum test speed
#define SPEED_STEP          20        // Speed step
int right = 10; 
int left = 10 ; 
// Create a Hardware Serial instance
HardwareSerial HoverSerial(2); // Use UART2 (GPIO16=RX, GPIO17=TX)

// Global variables
uint8_t idx = 0;
uint16_t bufStartFrame;
byte *p;
byte incomingByte;
byte incomingBytePrev;

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
// wifi
#include <WiFi.h>
#include <WebServer.h>

// Wi-Fi credentials
const char* ssid = "hey";
const char* password = "159357123";

WebServer server(80);


// Function to parse and split the string
void processData(String data) {
  int separatorIndex = data.indexOf('-');

  if (separatorIndex != -1) {
    String rightStr = data.substring(0, separatorIndex);
    String leftStr = data.substring(separatorIndex + 1);

     right = rightStr.toInt();
     left = leftStr.toInt();

    Serial.print("Right: ");
    Serial.println(right);
    Serial.print("Left: ");
    Serial.println(left);
  } else {
    Serial.println("Invalid format. Expected format: <right>-<left>");
  }
}

// Handle POST data
void handlePostData() {
  if (server.hasArg("plain")) {
    String data = server.arg("plain");
    Serial.println("Received data: " + data);

    processData(data);  // Call parsing function

    server.send(200, "text/plain", "Data received and processed");
  } else {
    server.send(400, "text/plain", "No data received");
  }
}




void setup() {
  //Serial.begin(115200);
  Serial.begin(SERIAL_BAUD); // Debugging
    HoverSerial.begin(HOVER_SERIAL_BAUD, SERIAL_8N1, 16, 17); // RX=GPIO16, TX=GPIO17
    Serial.println("Hoverboard Serial v1.0 (ESP32)");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", []() {
    server.send(200, "text/plain", "connected");
  });

  server.on("/post-data", HTTP_POST, handlePostData);
  server.begin();
}

// ########################## SEND ##########################
void Send(int16_t uSteer, int16_t uSpeed) {
    Command.start    = START_FRAME;
    Command.steer    = uSteer;
    Command.speed    = uSpeed;
    Command.checksum = (Command.start ^ Command.steer ^ Command.speed);

    HoverSerial.write((uint8_t*)&Command, sizeof(Command)); 
}

// ########################## RECEIVE ##########################
void Receive() {
    if (HoverSerial.available()) {
        incomingByte = HoverSerial.read();
        bufStartFrame = ((uint16_t)(incomingByte) << 8) | incomingBytePrev;
    } else {
        return;
    }

    if (bufStartFrame == START_FRAME) {
        p = (byte*)&NewFeedback;
        *p++ = incomingBytePrev;
        *p++ = incomingByte;
        idx = 2;
    } else if (idx >= 2 && idx < sizeof(SerialFeedback)) {
        *p++ = incomingByte;
        idx++;
    }

    if (idx == sizeof(SerialFeedback)) {
        uint16_t checksum = (NewFeedback.start ^ NewFeedback.cmd1 ^ NewFeedback.cmd2 ^ NewFeedback.speedR_meas 
                            ^ NewFeedback.speedL_meas ^ NewFeedback.batVoltage ^ NewFeedback.boardTemp ^ NewFeedback.cmdLed);

        if (NewFeedback.start == START_FRAME && checksum == NewFeedback.checksum) {
            memcpy(&Feedback, &NewFeedback, sizeof(SerialFeedback));
            Serial.print("speed: ");   Serial.print(Feedback.cmd1);
            Serial.print(" steering: ");  Serial.print(Feedback.cmd2);
            Serial.print(" speed_right: ");  Serial.print(Feedback.speedR_meas);
            Serial.print(" speed_left: ");  Serial.print(Feedback.speedL_meas);
            Serial.print(" battery_voltage: ");  Serial.print(Feedback.batVoltage);
            Serial.print(" board_temp: ");  Serial.println(Feedback.boardTemp);
            //Serial.print(" 7: ");  Serial.println(Feedback.cmdLed);
        } else {
            Serial.println("Invalid data skipped");
        }
        idx = 0;
    }

    incomingBytePrev = incomingByte;
}

// ########################## LOOP ##########################
unsigned long iTimeSend = 0;
int iTest = 200;
int iStep = SPEED_STEP;


void loop() {
    server.handleClient();
    unsigned long timeNow = millis();
    Receive();
    int speed = (left+right )/2 ; 
    int steering = (left -speed )*2 ; 

    if (iTimeSend > timeNow) return;
    iTimeSend = timeNow + TIME_SEND;
    
    Send(steering, speed);
/*
    iTest += iStep;
    if (iTest >= SPEED_MAX_TEST || iTest <= -SPEED_MAX_TEST)
        iStep = -iStep;
        */

    //digitalWrite(LED_BUILTIN, (timeNow % 2000) < 1000);
}
