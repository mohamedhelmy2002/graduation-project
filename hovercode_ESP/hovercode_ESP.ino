#include <HardwareSerial.h>

#define HOVER_SERIAL_BAUD   115200    // Baud rate for Hoverboard
#define SERIAL_BAUD         115200    // Baud rate for debugging
#define START_FRAME         0xABCD    // Start frame for reliable communication
#define TIME_SEND           100       // Sending time interval
#define SPEED_MAX_TEST      300       // Maximum test speed
#define SPEED_STEP          20        // Speed step

unsigned long iTimeSend = 0;

int iTest = 200;
int iStep = SPEED_STEP;
// geting distance
float wheel_dia = 16.51 ; // in cm 
float sp_r , sp_l = 0 ; // cm/s right and left 
float dist_r ,dist_l = 0 ; // distance in cm left and right  
// control 
int speed_right = 100 ; // negtave here is forward
int speed_left = 100 ; // postive here is
int sp = 0 ; 
int st = 0 ; 

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
            Serial.print("steering: ");   Serial.print(Feedback.cmd1);
            Serial.print(" speed: ");  Serial.print(Feedback.cmd2);
            sp_r= Feedback.speedR_meas ; 
            Serial.print(" sp_r (RPM): ");  Serial.print(sp_r);
            sp_l= Feedback.speedL_meas ;
            Serial.print(" sp_l (RPM): ");  Serial.print(sp_l);
            Serial.print(" battery_voltage: ");  Serial.print(Feedback.batVoltage);
            Serial.print(" board_temp: ");  Serial.println(Feedback.boardTemp);
            //Serial.print(" 7: ");  Serial.println(Feedback.cmdLed);
           // distance ();
        } else {
            Serial.println("Invalid data skipped");
        }
        idx = 0;
    }

    incomingBytePrev = incomingByte;
}
unsigned long timeNow = 0 ;
unsigned long delta = 0 ;

// ########################## SETUP ##########################
void setup() {
    Serial.begin(SERIAL_BAUD); // Debugging
    HoverSerial.begin(HOVER_SERIAL_BAUD, SERIAL_8N1, 16, 17); // RX=GPIO16, TX=GPIO17
    Serial.println("Hoverboard Serial v1.0 (ESP32)");
  //  pinMode(LED_BUILTIN, OUTPUT);
   timeNow= millis();
}
void distance (){ // any measurments you want to get MAKE IT IN RECIEVE 
  sp_l = sp_l*3.1416*wheel_dia/60;
  sp_r = sp_r*3.1416*wheel_dia/60; 
  dist_l = dist_l+(sp_l*delta/1000) ;  
  dist_r = dist_r+(sp_r*delta/1000) ;  
  Serial.print(" sp_r(cm/s)= ") ; Serial.print(sp_r); Serial.print(" sp_l (cm/s)= ") ; Serial.print(sp_l );
  Serial.print(" dist_r (cm)= ") ; Serial.print(dist_r); Serial.print(" dist_L (cm)= ") ; Serial.println(dist_l );
}
// ########################## LOOP ##########################

void loop() {
    delta = millis()-timeNow;
    Receive(); 
    if (iTimeSend > millis()) return;
    iTimeSend = millis() + TIME_SEND;
    // start control 
    

    Send(0,10 );

    timeNow = millis(); 

}