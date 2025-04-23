// DC Motor1 Pins
const int dc1EnablePin = 10; // ENB
const int dc1Int3Pin = 9;    // IN3
const int dc1Int4Pin = 8;    // IN4

// DC Motor2 Pins
const int dc2EnablePin = 32; // ENB
const int dc2Int3Pin = 33;   // IN3
const int dc2Int4Pin = 34;   // IN4

// Cup Stepper Motor Pins
const int cupsStepPin = 3;
const int cupsDirPin = 4;

// IR Sensor Pin
const int irSensorPin = 36; // IR sensor connected to pin 36

// Fruit Stepper Motor Pins
const int bananaStepPin = 12;
const int bananaDirPin = 11;

const int strawberryStepPin = 5;
const int strawberryDirPin = 6;

const int appleStepPin = 2;
const int appleDirPin = 7;

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

  // Run cup stepper motor
  displayMessage("Running Cup motor...");
  delay(1000);
  runcupStepper(cupsStepPin, cupsDirPin, initialStepperSteps, HIGH);
  delay(1000);
  displayMessage("Cup motor done.");

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
  if (digitalRead(irSensorPin) == LOW && !hasRunAfterIR) { // Object detected
    hasRunAfterIR = true;
    stopDCMotor1();

    displayMessage("Object detected. Stopping DC motors.");
    
   // stopDCMotor2();

   // delay(1000);  // Optional pause after stopping motors

    displayMessage("Running Banana Stepper...");
    runStepper(bananaStepPin, bananaDirPin, initialStepperSteps, HIGH);
    delay(2000);

    displayMessage("Running Strawberry Stepper...");
    runStepper(strawberryStepPin, strawberryDirPin, initialStepperSteps, HIGH);
    delay(2000);

    //  Start both DC motors BEFORE final stepper
    displayMessage("Starting both DC motors...");
    runDCMotor1Forward();
    runDCMotor2Forward();

    displayMessage("Running Apple Stepper...");
    runStepper(appleStepPin, appleDirPin, initialStepperSteps, HIGH);
    delay(2000);

    displayMessage("All steppers done. Letting motors run for 5 seconds...");
    //delay(5000); // Let DC motors continue running

    stopDCMotor1();
    runDCMotor2Forward();
    delay(4000);
    stopDCMotor2();
    displayMessage("DC Motors stopped.");
  }
   if (digitalRead(irSensorPin) == HIGH) {
    hasRunAfterIR = false;
  }
  // Reset the trigger after the object leaves the sensor area
  
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
