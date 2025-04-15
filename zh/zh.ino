// DC Motor Pins
const int dcEnablePin = 10; // ENB
const int dcInt3Pin = 9;    // IN3
const int dcInt4Pin = 8;    // IN4

// Cup Stepper Motor Pins
const int cupsStepPin = 3;
const int cupsDirPin = 4;
const int cupsEnablePin = 5;

// IR Sensor Pin
const int irSensorPin = 36; // IR sensor connected to pin 36

// Stepper Motor Pins
const int bananaStepPin = 12;
const int bananaDirPin = 13;

const int strawberryStepPin = 5;
const int strawberryDirPin = 6;

const int appleStepPin = 2;
const int appleDirPin = 7;

// Stepper motor settings
const int initialStepperSteps = 200;
const int stepDelaycup = 3000;
const int stepDelay = 1000;

bool hasRunAfterIR = false;  // Ensure stepper sequence only runs once per detection

void displayMessage(const char *message) {
  Serial.println(message);
}

void setup() {
  Serial.begin(9600);

  // Set DC motor pins
  pinMode(dcEnablePin, OUTPUT);
  pinMode(dcInt3Pin, OUTPUT);
  pinMode(dcInt4Pin, OUTPUT);

  // Set stepper motor pins
  pinMode(cupsStepPin, OUTPUT);
  pinMode(cupsDirPin, OUTPUT);
  pinMode(cupsEnablePin, OUTPUT);

  pinMode(bananaStepPin, OUTPUT);
  pinMode(bananaDirPin, OUTPUT);
  pinMode(strawberryStepPin, OUTPUT);
  pinMode(strawberryDirPin, OUTPUT);
  pinMode(appleStepPin, OUTPUT);
  pinMode(appleDirPin, OUTPUT);

  // Set IR sensor pin
  pinMode(irSensorPin, INPUT);

  // Run cup stepper motor
  displayMessage("Running Cup motor...");
  delay(1000);
  runcupStepper(cupsStepPin, cupsDirPin, initialStepperSteps, HIGH);
  delay(1000);
  displayMessage("Cup motor done.");

  // Run DC motor in forward direction
  displayMessage("Running DC motor forward...");
  runDCMotorForward();
  delay(5000); // Run for 5 seconds in forward direction
  stopDCMotor();
  displayMessage("DC motor stopped.");
}

void loop() {
  if (digitalRead(irSensorPin) == LOW)   // Object detected
    displayMessage("Object detected. Stopping DC motor.");
    stopDCMotor();
      

    
      delay(1000);  // Optional pause after stopping motor

      displayMessage("Running Banana Stepper...");
      runStepper(bananaStepPin, bananaDirPin, initialStepperSteps, HIGH);
      delay(2000);

      displayMessage("Running Strawberry Stepper...");
      runStepper(strawberryStepPin, strawberryDirPin, initialStepperSteps, HIGH);
      delay(2000);

      displayMessage("Running Apple Stepper...");
      runStepper(appleStepPin, appleDirPin, initialStepperSteps, HIGH);
      delay(2000);

      
      displayMessage("All steppers done.");
    
      runDCMotorForward();
      delay(5000); // Run for 5 seconds in forward direction
      stopDCMotor();
   
}

void runcupStepper(int stepPin, int dirPin, int steps, bool direction) {
  digitalWrite(dirPin, direction);
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelaycup);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelaycup);
  }
}

void runStepper(int stepPin, int dirPin, int steps, bool direction) {
  digitalWrite(dirPin, direction);
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
  }
}

void runDCMotorReverse() {
  digitalWrite(dcInt3Pin, HIGH);
  digitalWrite(dcInt4Pin, LOW);
  analogWrite(dcEnablePin, 255);
}

void runDCMotorForward() {
  digitalWrite(dcInt3Pin, LOW);
  digitalWrite(dcInt4Pin, HIGH);
  analogWrite(dcEnablePin, 255);
}

void stopDCMotor() {
  digitalWrite(dcInt3Pin, LOW);
  digitalWrite(dcInt4Pin, LOW);
  analogWrite(dcEnablePin, 0);
}
