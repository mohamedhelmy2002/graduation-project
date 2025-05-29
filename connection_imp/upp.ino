#define HS_U 14  // GPIO14 = D5   (Green cable)
#define HS_V 12  // GPIO12 = D6   (Yellow cable)
#define HS_W 13  // GPIO13 = D7   (Blue cable)

// Function prototype for the interrupt routine
void IRAM_ATTR hallInterrupt();

volatile byte lastcode = 0;
volatile byte newcode = 0;
volatile int pulseCounter = 0;

byte lastInc[] = {0, 5, 3, 1, 6, 4, 2};  // Forward rotation
byte lastDec[] = {0, 3, 6, 2, 5, 1, 4};  // Backward rotation

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(HS_U, INPUT_PULLUP);
  pinMode(HS_V, INPUT_PULLUP);
  pinMode(HS_W, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(HS_U), hallInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(HS_V), hallInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(HS_W), hallInterrupt, CHANGE);

  lastcode = readHallState();
}

void loop() {
  Serial.print("U: "); Serial.print(digitalRead(HS_U));
  Serial.print(" | V: "); Serial.print(digitalRead(HS_V));
  Serial.print(" | W: "); Serial.print(digitalRead(HS_W));
  Serial.print(" || Count: ");
  Serial.println(pulseCounter);

  delay(100);
}

byte readHallState() {
  return digitalRead(HS_U) | (digitalRead(HS_V) << 1) | (digitalRead(HS_W) << 2);
}

void IRAM_ATTR hallInterrupt() {
  newcode = readHallState();
  if (lastcode == lastInc[newcode]) {
    pulseCounter--;
    lastcode = newcode;
  } else if (lastcode == lastDec[newcode]) {
    pulseCounter++;
    lastcode = newcode;
  }
}
