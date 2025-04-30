// DC Motor1 Pins
const int dc1EnablePin = 10; // ENB
const int dc1Int3Pin = 9;    // IN3
const int dc1Int4Pin = 8;    // IN4

// Milk Pump Pins (H-Bridge)
const int pumpEnablePin = 40; // ENB
const int pumpInt3Pin = 41;   // IN3
const int pumpInt4Pin = 42;   // IN4

// Cleaning Pump Pins (H-Bridge)
const int cleaningEnablePin = 43; // ENA
const int cleaningInt1Pin = 44;   // IN1
const int cleaningInt2Pin = 45;   // IN2

// DC Motor2 Pins
const int dc2EnablePin = 32; // ENB
const int dc2Int3Pin = 33;   // IN3
const int dc2Int4Pin = 34;   // IN4

// Cup Stepper Motor Pins
const int cupsStepPin = 3;
const int cupsDirPin = 4;

// IR Sensor Pin
const int irSensorPin = 36; // IR sensor connected to pin 36

const int irSensor2Pin = 13; // Second IR sensor


// Fruit Stepper Motor Pins
const int bananaStepPin = 12;
const int bananaDirPin = 11;

const int strawberryStepPin = 5;
const int strawberryDirPin = 6;

const int appleStepPin = 2;
const int appleDirPin = 7;
// Mixer Pump Pins (H-Bridge)
const int mixerEnablePin = 37; // ENA
const int mixerInt1Pin = 38;   // IN1
const int mixerInt2Pin = 39;   // IN2


// Stepper motor settings
const int initialStepperSteps = 200;

const int stepDelaycup = 3000;
const int stepDelay = 2500;

bool hasRunAfterIR = false;  // To avoid repeating stepper actions on continuous detection

void displayMessage(const char *message) {
  Serial.println(message);
}

void setup() {
  Serial.begin(9600);

  // Set DC motor 1 pins
  pinMode(dc1EnablePin, OUTPUT);
  pinMode(dc1Int3Pin, OUTPUT);
  pinMode(dc1Int4Pin, OUTPUT);

  // Set DC motor 2 pins
  pinMode(dc2EnablePin, OUTPUT);
  pinMode(dc2Int3Pin, OUTPUT);
  pinMode(dc2Int4Pin, OUTPUT);

  // Set stepper motor pins
  pinMode(cupsStepPin, OUTPUT);
  pinMode(cupsDirPin, OUTPUT);

  pinMode(bananaStepPin, OUTPUT);
  pinMode(bananaDirPin, OUTPUT);
  pinMode(strawberryStepPin, OUTPUT);
  pinMode(strawberryDirPin, OUTPUT);
  pinMode(appleStepPin, OUTPUT);
  pinMode(appleDirPin, OUTPUT);

  // Set IR sensor pin
  pinMode(irSensorPin, INPUT);

  pinMode(irSensor2Pin, INPUT);


  // Run cup stepper motor
  displayMessage("Running Cup motor...");
  delay(1000);
  runcupStepper(cupsStepPin, cupsDirPin, initialStepperSteps, HIGH);
  delay(1000);
  displayMessage("Cup motor done.");
  // Set Milk Pump pins
  pinMode(pumpEnablePin, OUTPUT);
  pinMode(pumpInt3Pin, OUTPUT);
  pinMode(pumpInt4Pin, OUTPUT);

  // Set Mixer Pump pins
  pinMode(mixerEnablePin, OUTPUT);
  pinMode(mixerInt1Pin, OUTPUT);
  pinMode(mixerInt2Pin, OUTPUT);
  // Set Cleaning Pump pins
  pinMode(cleaningEnablePin, OUTPUT);
  pinMode(cleaningInt1Pin, OUTPUT);
  pinMode(cleaningInt2Pin, OUTPUT);


  // Run DC motors forward
  displayMessage("Running DC Motor 1 forward...");
  runDCMotor1Forward();
  delay(5000);
  stopDCMotor1();
  displayMessage("DC Motor 1 stopped.");

  // displayMessage("Running DC Motor 2 forward...");
  // runDCMotor2Forward();
  // delay(5000);
  // stopDCMotor2();
  // displayMessage("DC Motor 2 stopped.");
}

void loop() {

  // Original IR sensor logic
  if (digitalRead(irSensorPin) == LOW && !hasRunAfterIR) {
    hasRunAfterIR = true;
    stopDCMotor1();

    displayMessage("Object detected. Stopping DC motors.");
    
    displayMessage("Running Banana Stepper...");
    runStepper(bananaStepPin, bananaDirPin, initialStepperSteps, HIGH);
    delay(2000);

    displayMessage("Running Strawberry Stepper...");
    runStepper(strawberryStepPin, strawberryDirPin, initialStepperSteps, HIGH);
    delay(2000);

    // displayMessage("Starting both DC motors...");
    // runDCMotor1Forward();
    // runDCMotor2Forward();

    displayMessage("Running Apple Stepper...");
    runStepper(appleStepPin, appleDirPin, initialStepperSteps, HIGH);
    delay(2000);

    //displayMessage("All steppers done. Letting motors run for 5 seconds...");
    stopDCMotor1();
    delay(4000);
    displayMessage("DC Motors stopped.");

    displayMessage("Running Milk Pump...");
    runPump();
    delay(5000);
    stopPump();
    displayMessage("Milk Pump stopped.");

    displayMessage("Running Mixer Pump...");
    runMixerPump();
    delay(5000);
    stopMixerPump();
    displayMessage("Mixer Pump stopped.");
    runDCMotor1Forward();

      // New IR sensor logic
    if (digitalRead(irSensor2Pin) == LOW) { // Object detected by second IR sensor
      displayMessage("Second IR Sensor triggered. Stopping DC Motor 1, Running DC Motor 2...");
      stopDCMotor1();
      runDCMotor2Forward();
      delay(5000);
      stopDCMotor2();
    }
  }

  if (digitalRead(irSensorPin) == HIGH) {
    hasRunAfterIR = false;
  }
}



// CUP stepper motor function
void runcupStepper(int stepPin, int dirPin, int steps, bool direction) {
  digitalWrite(dirPin, direction);
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelaycup);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelaycup);
  }
}

// Fruit stepper motors function
void runStepper(int stepPin, int dirPin, int steps, bool direction) {
  digitalWrite(dirPin, direction);
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
  }
}

// DC Motor 1 controls
void runDCMotor1Forward() {
  digitalWrite(dc1Int3Pin, LOW);
  digitalWrite(dc1Int4Pin, HIGH);
  analogWrite(dc1EnablePin, 255);
}

void stopDCMotor1() {
  digitalWrite(dc1Int3Pin, LOW);
  digitalWrite(dc1Int4Pin, LOW);
  analogWrite(dc1EnablePin, 0);
}

// DC Motor 2 controls
void runDCMotor2Forward() {
  digitalWrite(dc2Int3Pin, LOW);
  digitalWrite(dc2Int4Pin, HIGH);
  analogWrite(dc2EnablePin, 255);
}

void stopDCMotor2() {
  digitalWrite(dc2Int3Pin, LOW);
  digitalWrite(dc2Int4Pin, LOW);
  analogWrite(dc2EnablePin, 0);
}
void runPump() {
  digitalWrite(pumpInt3Pin, LOW);
  digitalWrite(pumpInt4Pin, HIGH);
  analogWrite(pumpEnablePin, 255);
}

void stopPump() {
  digitalWrite(pumpInt3Pin, LOW);
  digitalWrite(pumpInt4Pin, LOW);
  analogWrite(pumpEnablePin, 0);
}
void runMixerPump() {
  digitalWrite(mixerInt1Pin, LOW);
  digitalWrite(mixerInt2Pin, HIGH);
  analogWrite(mixerEnablePin, 255);
}

void stopMixerPump() {
  digitalWrite(mixerInt1Pin, LOW);
  digitalWrite(mixerInt2Pin, LOW);
  analogWrite(mixerEnablePin, 0);
}
void runCleaningPump() {
  digitalWrite(cleaningInt1Pin, LOW);
  digitalWrite(cleaningInt2Pin, HIGH);
  analogWrite(cleaningEnablePin, 255);
}

void stopCleaningPump() {
  digitalWrite(cleaningInt1Pin, LOW);
  digitalWrite(cleaningInt2Pin, LOW);
  analogWrite(cleaningEnablePin, 0);
}

