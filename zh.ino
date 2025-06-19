#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <Servo.h>
#include <SoftwareSerial.h>

// --- Peripherals ---
Servo myServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- State Machine ---
enum State {
  LOGIN_PAGE,
  MENU,
  PROCESS
};
State currentState = LOGIN_PAGE;

// --- Order Variables ---
int pistachioSelectedOption = 0;
int juiceSelectedOption = 0;

// --- Web Order Handling ---
String espBuffer = "";
bool orderFromWeb = false;
int receivedOrderId = -1;
int receivedToppingsId = -1;

// --- Keypad Setup ---
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  { '1', '2', '3', '+' },
  { '4', '5', '6', '-' },
  { '7', '8', '9', 'x' },
  { '.', '0', '=', '%' }
};
byte rowPins[ROWS] = { 24, 25, 26, 27 };
byte colPins[COLS] = { 28, 29, 30, 31 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// --- ESP32 Communication ---
#define ESP_RX 50
#define ESP_TX 51
SoftwareSerial espSerial(ESP_RX, ESP_TX); // RX, TX

// --- Motor & Relay Pins ---
// DC Motor 1 (Conveyor)
const int dc1EnablePin = 10;
const int dc1Int3Pin = 9;
const int dc1Int4Pin = 8;

// DC Motor 2 (Press)
const int dc2EnablePin = 32;
const int dc2Int3Pin = 33;
const int dc2Int4Pin = 34;

// Milk/Water Pump (H-Bridge)
const int pumpEnablePin = 40;
const int pumpInt3Pin = 41;
const int pumpInt4Pin = 42;

// Cleaning Pump (H-Bridge)
const int cleaningEnablePin = 43;
const int cleaningInt1Pin = 44;
const int cleaningInt2Pin = 45;

// Mixer Pump (H-Bridge)
const int mixerEnablePin = 37;
const int mixerInt1Pin = 38;
const int mixerInt2Pin = 39;

// Stepper Motors
const int cupsStepPin = 3;
const int cupsDirPin = 4;
const int strawberryStepPin = 5;
const int strawberryDirPin = 6;
const int appleStepPin = 2;
const int appleDirPin = 7;
const int mangoStepPin = 22;
const int mangoDirPin = 23;
const int pistachioStepPin = 12;
const int pistachioDirPin = 11;

// --- Sensor Pins ---
const int irSensor1Pin = 36;  // Mixer station
const int irSensor2Pin = 13;  // Press machine
const int irSensor3Pin = 46;  // Pistachio station

// --- Relay Pin ---
const int relayPin = 35; // Mixer

// --- Constants ---
#define USERNAME_LEN 10
#define PASSWORD_LEN 4
const int initialStepperSteps = 105;
const int initialStepperCupSteps = 200;
const int stepDelaycup = 3000;
const int stepDelay = 2500;


// =================================================================
// SETUP FUNCTION
// =================================================================
void setup() {
  Serial.begin(9600);
  // ** IMPORTANT: Baud rate must match the ESP32's Serial2 speed **
  espSerial.begin(9600);

  lcd.init();
  lcd.backlight();
  myServo.attach(A0);

  // Initialize all pins
  pinMode(dc1EnablePin, OUTPUT);
  pinMode(dc1Int3Pin, OUTPUT);
  pinMode(dc1Int4Pin, OUTPUT);
  pinMode(dc2EnablePin, OUTPUT);
  pinMode(dc2Int3Pin, OUTPUT);
  pinMode(dc2Int4Pin, OUTPUT);
  pinMode(cupsStepPin, OUTPUT);
  pinMode(cupsDirPin, OUTPUT);
  pinMode(pistachioStepPin, OUTPUT);
  pinMode(pistachioDirPin, OUTPUT);
  pinMode(strawberryStepPin, OUTPUT);
  pinMode(strawberryDirPin, OUTPUT);
  pinMode(appleStepPin, OUTPUT);
  pinMode(appleDirPin, OUTPUT);
  pinMode(mangoStepPin, OUTPUT);
  pinMode(mangoDirPin, OUTPUT);
  pinMode(irSensor1Pin, INPUT);
  pinMode(irSensor2Pin, INPUT);
  pinMode(irSensor3Pin, INPUT);
  pinMode(pumpEnablePin, OUTPUT);
  pinMode(pumpInt3Pin, OUTPUT);
  pinMode(pumpInt4Pin, OUTPUT);
  pinMode(mixerEnablePin, OUTPUT);
  pinMode(mixerInt1Pin, OUTPUT);
  pinMode(mixerInt2Pin, OUTPUT);
  pinMode(cleaningEnablePin, OUTPUT);
  pinMode(cleaningInt1Pin, OUTPUT);
  pinMode(cleaningInt2Pin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH); // Start with relay off

  Serial.println("Vending Machine Ready. Listening for web orders...");
  displayLoginScreen();
}

// =================================================================
// MAIN LOOP
// =================================================================
void loop() {
  // 1. ALWAYS check for an incoming web order first.
  checkForWebOrder();

  // 2. If a web order was received, handle it immediately.
  if (orderFromWeb) {
    // Set default pistachio option for all web orders
    pistachioSelectedOption = receivedToppingsId;
    // The juice ID comes directly from the web order
    juiceSelectedOption = receivedOrderId;
    // Change state to start processing
    currentState = PROCESS;
    // Reset the flag so we don't process it again
    orderFromWeb = false;
  }

  // 3. If no web order, proceed with the normal state machine (keypad/LCD)
  switch (currentState) {
    case LOGIN_PAGE:
      handleLoginPage();
      break;

    case MENU:
      handleKeypadPistachioMode(); // This will lead to juice selection and then set state to PROCESS
      break;

    case PROCESS:
      handleProcess();

      currentState = LOGIN_PAGE;
      displayLoginScreen();
      break;
  }
}

// =================================================================
// SERIAL COMMUNICATION
// =================================================================

/**
 * @brief Checks for incoming data from ESP32, parses it, and sets the web order flag.
 * This function is non-blocking.
 */
void checkForWebOrder() {
  while (espSerial.available() > 0) {
    char c = espSerial.read();
    if (c == '\n') {
      espBuffer.trim();
      if (espBuffer.length() > 0) {
        int commaIndex = espBuffer.indexOf(',');
        if (commaIndex > 0) {
          String orderPart = espBuffer.substring(0, commaIndex);
          String toppingsPart = espBuffer.substring(commaIndex + 1);

          receivedOrderId = orderPart.toInt();
          receivedToppingsId = toppingsPart.toInt();

          if (receivedOrderId > 0 && receivedToppingsId >= 0) {
            orderFromWeb = true;
            Serial.print("📦 Web Order ID: ");
            Serial.print(receivedOrderId);
            Serial.print(", 🍫 Toppings ID: ");
            Serial.println(receivedToppingsId);
          } else {
            Serial.println("❌ Invalid data received: " + espBuffer);
          }
        } else {
          Serial.println("❌ Missing comma in payload: " + espBuffer);
        }
      }
      espBuffer = "";
    } else {
      espBuffer += c;
    }
  }
}


// =================================================================
// STATE HANDLING & UI
// =================================================================

void displayMessage(const char* message) {
  Serial.println(message);
  // Optionally, you can also display temporary messages on the LCD here
}

void displayLoginScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1:Login");
  lcd.setCursor(0, 1);
  lcd.print("2:SignUp");
}

void handleLoginPage() {
  char key = keypad.getKey();
  if (key == '1') {
    login();
  } else if (key == '2') {
    signup();
  }
}

void handleKeypadPistachioMode() {
  lcd.clear();
  lcd.print("Bubbles Opt:1:Bo");
  lcd.setCursor(0,1);
  lcd.print("2:Af 3:Be 4:Non");
  displayMessage("Select bubbles 1-4");

  char pistachioKey = NO_KEY;
  while (pistachioKey == NO_KEY) {
    pistachioKey = keypad.getKey();

    checkForWebOrder();
    if(orderFromWeb) return; // If web order comes, exit immediately
  }

  if (pistachioKey >= '1' && pistachioKey <= '4') {
    pistachioSelectedOption = pistachioKey - '0';
    delay(200); // debounce
    handleKeypadJuiceMode();
  } else {
    displayMessage("Invalid! Press 1-4");
    delay(1000);
    // No state change
  }
}

void handleKeypadJuiceMode() {
  lcd.clear();
  lcd.print("Select Juice:");
  displayMessage("Select Juice 1-7");

  char juiceKey = NO_KEY;
  while (juiceKey == NO_KEY) {
    juiceKey = keypad.getKey();

    checkForWebOrder();
    if(orderFromWeb) return; // If web order comes, exit immediately
  }

  if (juiceKey >= '1' && juiceKey <= '7') {
    juiceSelectedOption = juiceKey - '0';
    delay(200); // debounce
    currentState = PROCESS; // Set state to start processing
  } else {
    displayMessage("Invalid! Press 1-7");
    delay(1000);
    // Return to pistachio selection
  }
}

void handleProcess() {
  lcd.clear();
  lcd.print("Preparing Order.");
  Serial.println("--- STARTING NEW ORDER ---");
  Serial.print("Juice ID: "); Serial.println(juiceSelectedOption);
  Serial.print("Pistachio Opt: "); Serial.println(pistachioSelectedOption);


  stopDCMotor1();
  displayMessage("Running Cup motor...");
  delay(1000);
  runcupStepper(cupsStepPin, cupsDirPin, initialStepperCupSteps, HIGH);
  delay(1000);
  runDCMotor1Forward();

  if (pistachioSelectedOption == 1 || pistachioSelectedOption == 3) {
    Serial.println("Pistachio Before...");
    PistachioSensorDetect();
  }

  runDCMotor1Forward();

  switch (juiceSelectedOption) {
    case 1:
      MixerSensorDetect();
      displayMessage("Running Strawberry Stepper...");
      runStepper(strawberryStepPin, strawberryDirPin, 125, HIGH);
      delay(2000);
      RunMixing();
      break;

    case 2:
      MixerSensorDetect();
      displayMessage("Running Apple Stepper...");
      runStepper(appleStepPin, appleDirPin, 80, HIGH);
      delay(2000);
      RunMixing();
      break;
    
    case 3:
      MixerSensorDetect();
      displayMessage("Running Mango Stepper...");
      runStepper(mangoStepPin, mangoDirPin, 125, HIGH);
      delay(2000);
      RunMixing();
      break;
    case 4:
      MixerSensorDetect();
      displayMessage("Running Strawberry Stepper...");
      runStepper(strawberryStepPin, strawberryDirPin, 110, HIGH);
      delay(2000);
      displayMessage("Running Apple Stepper...");
      runStepper(appleStepPin, appleDirPin, 80, HIGH);
      delay(2000);
      RunMixing();
      break;
    case 5:
      MixerSensorDetect();
      displayMessage("Running Strawberry Stepper...");
      runStepper(strawberryStepPin, strawberryDirPin, 110, HIGH);
      delay(2000);
      displayMessage("Running Mango Stepper...");
      runStepper(mangoStepPin, mangoDirPin, 110, HIGH);
      delay(2000);
      RunMixing();
      break;
    case 6:
      MixerSensorDetect();
      displayMessage("Running Apple Stepper...");
      runStepper(appleStepPin, appleDirPin, 80, HIGH);
      delay(2000);
      displayMessage("Running Mango Stepper...");
      runStepper(mangoStepPin, mangoDirPin, 110, HIGH);
      delay(2000);
      RunMixing();
      break;
    case 7:
      MixerSensorDetect();
      displayMessage("Running Strawberry Stepper...");
      runStepper(strawberryStepPin, strawberryDirPin, 110, HIGH);
      delay(2000);
      displayMessage("Running Apple Stepper...");
      runStepper(appleStepPin, appleDirPin, 80, HIGH);
      delay(2000);
      displayMessage("Running Mango Stepper...");
      runStepper(mangoStepPin, mangoDirPin, 110, HIGH);
      delay(2000);
      RunMixing();
      break;

    default:
      Serial.println("Error: Invalid Juice selection!");
      break;
  }

  if (pistachioSelectedOption == 1 || pistachioSelectedOption == 2) {
    Serial.println("Bubbels After...");
    runDCMotor1Backward();
    PistachioSensorDetect();
  }
  
  runDCMotor1Forward();
  PressMachineSensorDetect();
  delay(3000);
  runDCMotor1Forward();
  delay(5000);
  stopDCMotor1();

  lcd.clear();
  lcd.print("Order is ready!");
  Serial.println("--- ORDER COMPLETE ---");
  delay(3000);

  RunCleaning();
}


void getInput(char* buffer, const char* label, int maxLen) {
  lcd.clear();
  lcd.print(label);
  lcd.setCursor(0, 1);
  int index = 0;
  memset(buffer, 0, maxLen + 1); // Clear the buffer
  while (true) {
    char key = keypad.getKey();
    if (key) {
      if (key == '.') break; // Use '.' as the 'Enter' key
      if (index < maxLen) {
        buffer[index++] = key;
        lcd.print("*");
      }
    }
  }
}

void signup() {
  char username[USERNAME_LEN + 1];
  char password[PASSWORD_LEN + 1];

  getInput(username, "Set Username:", USERNAME_LEN);
  delay(500);
  getInput(password, "Set Password:", PASSWORD_LEN);

  for (int i = 0; i < USERNAME_LEN; i++) {
    EEPROM.write(i, username[i]);
  }
  for (int i = 0; i < PASSWORD_LEN; i++) {
    EEPROM.write(USERNAME_LEN + i, password[i]);
  }

  lcd.clear();
  lcd.print("Signup Complete!");
  delay(2000);
  currentState = LOGIN_PAGE;
  displayLoginScreen();
}



void login() {
  char username[USERNAME_LEN + 1];
  char password[PASSWORD_LEN + 1];
  char storedUser[USERNAME_LEN + 1];
  char storedPass[PASSWORD_LEN + 1];

  getInput(username, "Enter Username:", USERNAME_LEN);
  delay(500);
  getInput(password, "Enter Password:", PASSWORD_LEN);

  // Admin Login
  if (strcmp(username, "123") == 0 && strcmp(password, "123") == 0) {
    lcd.clear();
    lcd.print("Admin Access");
    delay(1000);

    while (true) {
      lcd.clear();
      lcd.print("1:C EEPROM");
      lcd.setCursor(0, 1);
      lcd.print("2:CA 3:GB");

      char choice = 0;
      while (!choice) {
        choice = keypad.getKey();
      }

      if (choice == '1') {
        for (int i = 0; i < EEPROM.length(); i++) {
          EEPROM.write(i, 0);
        }
        lcd.clear();
        lcd.print("EEPROM Cleared");
        delay(2000);

        lcd.clear();
        lcd.print("Returning...");
        delay(1500);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("1:Login");
        lcd.setCursor(0, 1);
        lcd.print("2:SignUp");
        currentState = LOGIN_PAGE;
        return;

      } else if (choice == '3') {
        lcd.clear();
        lcd.print("Returning...");
        delay(1000);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("1:Login");
        lcd.setCursor(0, 1);
        lcd.print("2:SignUp");
        currentState = LOGIN_PAGE;
        return;
      }
      else if (choice == '2') {
        RunAdminCleaning();
        
        lcd.clear();
        lcd.print("Returning...");
        delay(1000);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("1:Login");
        lcd.setCursor(0, 1);
        lcd.print("2:SignUp");
        currentState = LOGIN_PAGE;
        return;
      }
    }
  }

  // Read stored credentials
  for (int i = 0; i < USERNAME_LEN; i++) {
    storedUser[i] = EEPROM.read(i);
  }
  for (int i = 0; i < PASSWORD_LEN; i++) {
    storedPass[i] = EEPROM.read(USERNAME_LEN + i);
  }

  storedUser[USERNAME_LEN] = '\0';
  storedPass[PASSWORD_LEN] = '\0';

  if (strcmp(username, storedUser) == 0 && strcmp(password, storedPass) == 0) {
    lcd.clear();
    lcd.print("Access Granted");
    delay(2000);
    currentState = MENU;  // Move to main menu
  } else {
    lcd.clear();
    lcd.print("Access Denied");
    delay(2000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("1:Login");
    lcd.setCursor(0, 1);
    lcd.print("2:SignUp");
    currentState = LOGIN_PAGE;  // Stay on login screen
  }
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
  digitalWrite(dirPin, !direction);
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
  }
  digitalWrite(dirPin, direction);
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
  }
  digitalWrite(dirPin, !direction);
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
  }
}

void runDCMotor1Forward() {
  digitalWrite(dc1Int3Pin, LOW);
  digitalWrite(dc1Int4Pin, HIGH);
  analogWrite(dc1EnablePin, 255);
}

void runDCMotor1Backward() {
  digitalWrite(dc1Int3Pin, HIGH);
  digitalWrite(dc1Int4Pin, LOW);
  analogWrite(dc1EnablePin, 255);
}

void stopDCMotor1() {
  digitalWrite(dc1Int3Pin, LOW);
  digitalWrite(dc1Int4Pin, LOW);
  analogWrite(dc1EnablePin, 0);
}

void runDCMotor2Forward() {
  digitalWrite(dc2Int3Pin, LOW);
  digitalWrite(dc2Int4Pin, HIGH);
  analogWrite(dc2EnablePin, 255);
}
void runDCMotor2Backward() {
  digitalWrite(dc2Int3Pin, HIGH);
  digitalWrite(dc2Int4Pin, LOW);
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

void turnRelayOn() {
  digitalWrite(relayPin, LOW);
}

void turnRelayOff() {
  digitalWrite(relayPin, HIGH);
}

void PistachioSensorDetect() {
  while (digitalRead(irSensor3Pin) == HIGH) {
    // Keep conveyor running until sensor detects the cup
    // Add a tiny delay to prevent bus-waiting
    delay(1);
  }
  delay(1000);
  stopDCMotor1();
  displayMessage("Running Pistachio Stepper...");
  runStepperPistachio(pistachioStepPin, pistachioDirPin, 160, HIGH);
  delay(2000);
}

void MixerSensorDetect() {
  while (digitalRead(irSensor1Pin) == HIGH) {
    delay(1);
  }
  delay(1050);
  stopDCMotor1();
}

void PressMachineSensorDetect() {
  while (digitalRead(irSensor2Pin) == HIGH) {
    delay(1);
  }
  delay(1000);
  stopDCMotor1();
  displayMessage("Running press machine DC motor...");
  runDCMotor2Forward();
  delay(4000);
  stopDCMotor2();

  delay(1000);
  runDCMotor2Backward();
  delay(500);
  stopDCMotor2();
}

void RunMixing() {
  stopDCMotor1();

  delay(1000);
  displayMessage("Running water Pump...");
  runPump();
  delay(10000);
  stopPump();

  delay(1000);
  displayMessage("Turning on Mixer Relay...");
  turnRelayOn();
  delay(80000);
  turnRelayOff();

  delay(1000);
  displayMessage("Running Cup fill Pump...");
  runCleaningPump();
  delay(15000);
  stopCleaningPump();
}

void runStepperPistachio(int stepPin, int dirPin, int steps, bool direction) {
  digitalWrite(dirPin, direction);
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
  }
}

void RunCleaning() {

  lcd.clear();
  lcd.print("Cleaning cycle..");
  displayMessage("Start cleaning");
  displayMessage("Running water Pump...");
  runPump();
  delay(8000);
  stopPump();
  displayMessage("Turning on Relay...");
  turnRelayOn();
  delay(9000);
  turnRelayOff();
  delay(1000);
  displayMessage("Running cleaning Pump...");
  runMixerPump();
  delay(12000);
  stopMixerPump();
  delay(1000);
  displayMessage("Done mixer cleaning");

  for (int pos = 30; pos >= 0; pos--) {
    myServo.write(pos);
    delay(15);
  }

  runDCMotor1Forward();
  delay(18000);
  stopDCMotor1();

  for (int pos = 0; pos <= 30; pos++) {
    myServo.write(pos);
    delay(15);
  }

  lcd.clear();
  lcd.print("Cleaning done.");
  delay(2000);
}

void RunAdminCleaning() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("full Cleaning.");
  lcd.setCursor(0, 1);
  lcd.print("In process");


  stopDCMotor1();
  runcupStepper(cupsStepPin, cupsDirPin, initialStepperCupSteps, HIGH);
  runDCMotor1Forward();

  MixerSensorDetect();
  
  delay(1000);
  displayMessage("Running water Pump...");
  runPump();
  delay(8000);
  stopPump();

  delay(1000);
  displayMessage("Turning on Mixer Relay...");
  turnRelayOn();
  delay(6000);
  turnRelayOff();

  delay(1000);
  displayMessage("Running Cup fill Pump...");
  runCleaningPump();
  delay(13000);
  stopCleaningPump();

  runDCMotor1Forward();
  PressMachineSensorDetect();
  delay(2000);
  runDCMotor1Forward();
  delay(2000);

  for (int pos = 30; pos >= 0; pos--) {
    myServo.write(pos);
    delay(15);
  }

  runDCMotor1Forward();
  delay(18000);
  stopDCMotor1();

  for (int pos = 0; pos <= 30; pos++) {
    myServo.write(pos);
    delay(15);
  }

  lcd.clear();
  lcd.print("Cleaning done.");
  delay(2000);

}
