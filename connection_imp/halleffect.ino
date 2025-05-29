#define U_1 14  // GPIO14 = D5  (Green cable)
#define V_1 12  // GPIO12 = D6 (Yellow cable)
#define W_1 13  // GPIO13 = D7  (Blue cable)


#define U_2 5   // GPIO5  = D1  (Green cable) 
#define V_2 4   // GPIO4  = D2 (Yellow cable)
#define W_2 0   // GPIO0  = D3  (Blue cable)


// Function prototypes
void IRAM_ATTR hallInterrupt1();
void IRAM_ATTR hallInterrupt2();

volatile byte lastcode_1 = 0, newcode_1 = 0;
volatile byte lastcode_2 = 0, newcode_2 = 0;
volatile int c_1 = 0;
volatile int c_2 = 0;

byte lastInc[] = {0, 5, 3, 1, 6, 4, 2};  // Forward rotation
byte lastDec[] = {0, 3, 6, 2, 5, 1, 4};  // Backward rotation

void setup() {
  Serial.begin(115200);
  delay(100);

  // Setup wheel 1 pins
  pinMode(U_1, INPUT_PULLUP);
  pinMode(V_1, INPUT_PULLUP);
  pinMode(W_1, INPUT_PULLUP);

  // Setup wheel 2 pins
  pinMode(U_2, INPUT_PULLUP);
  pinMode(V_2, INPUT_PULLUP);
  pinMode(W_2, INPUT_PULLUP);

  // Attach interrupts for wheel 1
  attachInterrupt(digitalPinToInterrupt(U_1), hallInterrupt1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(V_1), hallInterrupt1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(W_1), hallInterrupt1, CHANGE);

  // Attach interrupts for wheel 2
  attachInterrupt(digitalPinToInterrupt(U_2), hallInterrupt2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(V_2), hallInterrupt2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(W_2), hallInterrupt2, CHANGE);

  lastcode_1 = readHallState(U_1, V_1, W_1);
  lastcode_2 = readHallState(U_2, V_2, W_2);
}

void loop() {
  Serial.print("U1: "); Serial.print(digitalRead(U_1));
  Serial.print(" V1: "); Serial.print(digitalRead(V_1));
  Serial.print(" W1: "); Serial.print(digitalRead(W_1));
  Serial.print("| c1: "); Serial.print(c_1);

  Serial.print(" |U2: "); Serial.print(digitalRead(U_2));
  Serial.print(" V2: "); Serial.print(digitalRead(V_2));
  Serial.print(" W2: "); Serial.print(digitalRead(W_2));
  Serial.print(" |c2: "); Serial.println(c_2);

  delay(100);
}

byte readHallState(int U_pin, int V_pin, int W_pin) {
  return digitalRead(U_pin) | (digitalRead(V_pin) << 1) | (digitalRead(W_pin) << 2);
}

void IRAM_ATTR hallInterrupt1() {
  newcode_1 = readHallState(U_1, V_1, W_1);
  if (lastcode_1 == lastInc[newcode_1]) {
    c_1--;
    lastcode_1 = newcode_1;
  } else if (lastcode_1 == lastDec[newcode_1]) {
    c_1++;
    lastcode_1 = newcode_1;
  }
}

void IRAM_ATTR hallInterrupt2() {
  newcode_2 = readHallState(U_2, V_2, W_2);
  if (lastcode_2 == lastInc[newcode_2]) {
    c_2--;
    lastcode_2 = newcode_2;
  } else if (lastcode_2 == lastDec[newcode_2]) {
    c_2++;
    lastcode_2 = newcode_2;
  }
}
