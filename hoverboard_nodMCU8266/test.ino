
// ########################## DEFINES ##########################
#define SERIAL_BAUD         115200      // [-] Baud rate for built-in Serial (used for the Serial Monitor)
#define START_FRAME         0xABCD     	// [-] Start frme definition for reliable serial communication
#define TIME_SEND           100         // [ms] Sending time interval
#define SPEED_MAX_TEST      300         // [-] Maximum speed for testing
#define SPEED_STEP          20          // [-] Speed step
// #define DEBUG_RX                        // [-] Debug received data. Prints all bytes to serial (comment-out to disable)

#include <SoftwareSerial.h>
//SoftwareSerial Serial(2,3);        // RX, TX
// Global variables
unsigned long iTimeSend = 0;
uint8_t idx = 0;                        // Index for new data pointer
uint16_t bufStartFrame;                 // Buffer Start Frame
byte *p;                                // Pointer declaration for the new received data
byte incomingByte;
byte incomingBytePrev;

typedef struct{
   uint16_t start;
   int16_t  steer;
   int16_t  speed;
   uint16_t checksum;
} SerialCommand;
SerialCommand Command;

typedef struct{
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
// ########################## SEND ##########################
void Send(int16_t uSteer, int16_t uSpeed)
{
  // Create command
  Command.start    = (uint16_t)START_FRAME;
  Command.steer    = (int16_t)uSteer;
  Command.speed    = (int16_t)uSpeed;
  Command.checksum = (uint16_t)(Command.start ^ Command.steer ^ Command.speed);

  // Write to Serial
  Serial.write((uint8_t *) &Command, sizeof(Command)); 
}

// ########################## RECEIVE ##########################
void Receive()
{
    // Check for new data availability in the Serial buffer
    if (Serial.available()) {
        incomingByte 	  = Serial.read();                                   // Read the incoming byte
        bufStartFrame	= ((uint16_t)(incomingByte) << 8) | incomingBytePrev;       // Construct the start frame
    }
    else {
        return;
    }

  // If DEBUG_RX is defined print all incoming bytes
  #ifdef DEBUG_RX
        Serial.print(incomingByte);
        return;
    #endif

    // Copy received data
    if (bufStartFrame == START_FRAME) {	                    // Initialize if new data is detected
        p       = (byte *)&NewFeedback;
        *p++    = incomingBytePrev;
        *p++    = incomingByte;
        idx     = 2;	
    } else if (idx >= 2 && idx < sizeof(SerialFeedback)) {  // Save the new received data
        *p++    = incomingByte; 
        idx++;
    }	
    
    // Check if we reached the end of the package
    if (idx == sizeof(SerialFeedback)) {
        uint16_t checksum;
        checksum = (uint16_t)(NewFeedback.start ^ NewFeedback.cmd1 ^ NewFeedback.cmd2 ^ NewFeedback.speedR_meas ^ NewFeedback.speedL_meas
                            ^ NewFeedback.batVoltage ^ NewFeedback.boardTemp ^ NewFeedback.cmdLed);

        // Check validity of the new data
        if (NewFeedback.start == START_FRAME && checksum == NewFeedback.checksum) {
            // Copy the new data
            memcpy(&Feedback, &NewFeedback, sizeof(SerialFeedback));

            // Print data to built-in Serial
            Serial.print("sp: ");   Serial.print(Feedback.cmd1);
            Serial.print("st: ");  Serial.print(Feedback.cmd2);
            Serial.print(" sp_r: ");  Serial.print(Feedback.speedR_meas);
            Serial.print(" sp_l: ");  Serial.print(Feedback.speedL_meas);
            Serial.print(" bat_V: ");  Serial.print(Feedback.batVoltage);
            Serial.print(" temp: ");  Serial.println(Feedback.boardTemp);
        } else {
          Serial.println("Non-valid data skipped");
        }
        idx = 0;    // Reset the index (it prevents to enter in this if condition in the next cycle)
    }

    // Update previous states
    incomingBytePrev = incomingByte;
}
//##########################comunication ######################
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
int speed=0 ; 
int steering=0 ; 

void loop(void)
{ 
  unsigned long timeNow = millis();

  // Check for new received data
  Receive();

  // Send commands


  // Blink the LED
  digitalWrite(LED_BUILTIN, (timeNow%2000)<1000);
  
  server.handleClient();

  // Update the variable that will be sent back to Python
 // dataToSend = "ESP counter: " + String(counter++);
  dataToSend = "sp: " + String(Feedback.cmd1) +
                    " st: " + String(Feedback.cmd2) +
                    " sp_r: " + String(Feedback.speedR_meas) +
                    " sp_l: " + String(Feedback.speedL_meas) +
                    " bat_V: " + String(Feedback.batVoltage) +
                    " temp: " + String(Feedback.boardTemp);

Serial.println(dataToSend);


  // Optional: print the received value every second
  if (millis() - lastPrintTime >= 150) {
    int separatorIndex = receivedFromPython.indexOf('-');
    steering = receivedFromPython.substring(0, separatorIndex).toInt();
    speed = receivedFromPython.substring(separatorIndex + 1).toInt();
    Serial.println("speed: " + receivedFromPython);
    lastPrintTime = millis();
  }
    if (iTimeSend > timeNow) return; // may nneed to remove
  iTimeSend = timeNow + TIME_SEND;
  Send(steering, speed);

  delay(10); // Small delay to keep it responsive


}

// ########################## END ##########################
