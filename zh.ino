#include <Keypad.h>               

#include <LiquidCrystal_I2C.h>  
#include <EEPROM.h>

enum State { loginPage ,MENU, PROCESS };
State state = loginPage;
int PistachioSelectedOption = 0;
int JuiceSelectedOption=0;

// Keypad setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', '+'},
  {'4', '5', '6', '-'},
  {'7', '8', '9', 'x'},
  {'.', '0', '=', '%'}
};
byte rowPins[ROWS] = {24, 25, 26, 27};
byte colPins[COLS] = {28, 29, 30, 31};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

LiquidCrystal_I2C lcd(0x27, 16, 2); 

// DC Motor1 Pins
const int dc1EnablePin = 10; // ENB
const int dc1Int3Pin = 9;    // IN3
const int dc1Int4Pin = 8;    // IN4

// DC Motor2 Pins
const int dc2EnablePin = 32; // ENB
const int dc2Int3Pin = 33;   // IN3
const int dc2Int4Pin = 34;   // IN4


// Milk Pump Pins (H-Bridge)
const int pumpEnablePin = 40; // ENB
const int pumpInt3Pin = 41;   // IN3
const int pumpInt4Pin = 42;   // IN4

// Cleaning Pump Pins (H-Bridge)
const int cleaningEnablePin = 43; // ENA
const int cleaningInt1Pin = 44;   // IN1
const int cleaningInt2Pin = 45;   // IN2

// Mixer Pump Pins (H-Bridge)
const int mixerEnablePin = 37; // ENA
const int mixerInt1Pin = 38;   // IN1
const int mixerInt2Pin = 39;   // IN2


// Cup Stepper Motor Pins
const int cupsStepPin = 3;
const int cupsDirPin = 4;
// strawberry Stepper Motor Pins
const int strawberryStepPin = 5;
const int strawberryDirPin = 6;
// apple Stepper Motor Pins
const int appleStepPin = 2;
const int appleDirPin = 7;
//Mango  stepper motor
const int mangoStepPin = 22;
const int mangoDirPin = 23;
// Pistachio Stepper Motor Pins
const int bistishioStepPin = 12;
const int bistishioDirPin = 11;



// Stepper motor settings
const int initialStepperSteps = 100;
const int initialStepperCupSteps = 200;
const int stepDelaycup = 3000;
const int stepDelay = 2500;


// IR Sensor Pin
const int irSensor1Pin = 36; // First IR sensor mixer
const int irSensor2Pin = 13; // Second IR sensor press machine
const int irSensor3Pin = 46; // third IR sensor bistashio


// mixer & pump relay 
const int relayPin = 35;
const int relayPumpPin = 47;


char Pistachiokey;
char JuiceKey;
char key;

#define USERNAME_LEN 10
#define PASSWORD_LEN 4

void displayMessage(const char *message) {
  Serial.println(message);
}

void setup() {
  Serial.begin(9600);

  lcd.init();            
  lcd.backlight();
  lcd.clear();       
  lcd.setCursor(0, 0);             
  lcd.print("1:Login");
  lcd.setCursor(0, 1);
  lcd.print("2:SignUp");

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

  pinMode(bistishioStepPin, OUTPUT);
  pinMode(bistishioDirPin, OUTPUT);

  pinMode(strawberryStepPin, OUTPUT);
  pinMode(strawberryDirPin, OUTPUT);

  pinMode(appleStepPin, OUTPUT);
  pinMode(appleDirPin, OUTPUT);

  pinMode(mangoStepPin, OUTPUT);
  pinMode(mangoDirPin, OUTPUT);

  // Set IR sensor pin
  pinMode(irSensor1Pin, INPUT);
  pinMode(irSensor2Pin, INPUT);
  pinMode(irSensor3Pin, INPUT);

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

  // mixer relay 
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH); // Start with relay off
  // Pump relay 
  pinMode(relayPumpPin, OUTPUT);
  digitalWrite(relayPumpPin, HIGH); // Start with relay off

}

void loop() {
  if(state == loginPage){
    key = keypad.getKey();   // Read a key
  if (key == '1') {
    login();
    state = MENU;
  } else if (key == '2') {
    signup();
  }
  }
  else if (state == MENU) {
    handleKeypadPistachioMode();
  } else if (state == PROCESS) {
    handleProcess();
    state = loginPage; // return to menu after process
  }
}

void handleKeypadPistachioMode() {
  lcd.clear();
  lcd.print("Enter your order");

  displayMessage("Select Pistachio 1-4");

  while (true) {
    Pistachiokey = keypad.getKey();
    if (Pistachiokey) {
      if (Pistachiokey >= '1' && Pistachiokey <= '4') {//1 both//2 after//3 before//4 nothing
        PistachioSelectedOption = Pistachiokey - '0';
        delay(200);  // debounce
        handleKeypadJuiceMode();
        break;
      } else {
        displayMessage("Invalid! Press 1-4");
        delay(1000);
        displayMessage("Select Pistachio 1-4");
      }
    }
  }
}

void handleKeypadJuiceMode() {
  displayMessage("Select Juice 1-7");

  while (true) {
    JuiceKey = keypad.getKey();
    if (JuiceKey) {
      if (JuiceKey >= '1' && JuiceKey <= '7') {
        JuiceSelectedOption = JuiceKey - '0';
        delay(200);  // debounce
        state = PROCESS;
        break;
      } else {
        displayMessage("Invalid! Press 1-7");
        delay(1000);
        displayMessage("Select Juice 1-7");
      }
    }
  }
}

void handleProcess() {
  lcd.clear();
  lcd.print("Preparing Order.");
  
  stopDCMotor1();
  displayMessage("Running Cup motor...");
  delay(1000);
  runcupStepper(cupsStepPin, cupsDirPin, initialStepperCupSteps, HIGH);
  delay(1000);
  runDCMotor1Forward();

  if (PistachioSelectedOption == 1 || PistachioSelectedOption == 3) {
    Serial.println("Pistachio Before...");
    PistachioSensorDetect();
  }

  runDCMotor1Forward();
  
  switch (JuiceSelectedOption) {
    case 1:
      MixerSensorDetect();

      displayMessage("Running Strawberry Stepper...");
      runStepper(strawberryStepPin, strawberryDirPin, initialStepperSteps, HIGH);
      delay(2000);
      
      RunMixing();
      break;

    case 2:
      MixerSensorDetect();

      displayMessage("Running Apple Stepper...");
      runStepper(appleStepPin, appleDirPin, initialStepperSteps, HIGH);
      delay(2000);

      RunMixing();
      break;

    case 3:
      MixerSensorDetect();

      displayMessage("Running Mango Stepper...");
      runStepper(mangoStepPin, mangoDirPin, initialStepperSteps, HIGH);
      delay(2000);

      RunMixing();
      break;

    case 4:
      MixerSensorDetect();

      displayMessage("Running Strawberry Stepper...");
      runStepper(strawberryStepPin, strawberryDirPin, initialStepperSteps, HIGH);
      delay(2000);
    
      displayMessage("Running Apple Stepper...");
      runStepper(appleStepPin, appleDirPin, initialStepperSteps, HIGH);
      delay(2000);

      RunMixing();
      break;

    case 5:
      MixerSensorDetect();

      displayMessage("Running Apple Stepper...");
      runStepper(appleStepPin, appleDirPin, initialStepperSteps, HIGH);
      delay(2000);

      displayMessage("Running Mango Stepper...");
      runStepper(mangoStepPin, mangoDirPin, initialStepperSteps, HIGH);
      delay(2000);
      
      RunMixing();
      break;

    case 6:
      MixerSensorDetect();

      displayMessage("Running Apple Stepper...");
      runStepper(appleStepPin, appleDirPin, initialStepperSteps, HIGH);
      delay(2000);

      displayMessage("Running Mango Stepper...");
      runStepper(mangoStepPin, mangoDirPin, initialStepperSteps, HIGH);
      delay(2000);

      RunMixing();
      break;

    case 7:
    MixerSensorDetect();

    displayMessage("Running Strawberry Stepper...");
    runStepper(strawberryStepPin, strawberryDirPin, initialStepperSteps, HIGH);
    delay(2000);

    displayMessage("Running Apple Stepper...");
    runStepper(appleStepPin, appleDirPin, initialStepperSteps, HIGH);
    delay(2000);

    displayMessage("Running Mango Stepper...");
    runStepper(mangoStepPin, mangoDirPin, initialStepperSteps, HIGH);
    delay(2000);

    RunMixing();
      break;

    default:
      Serial.println("Somthing went wrong");
      break;
  }

  if (PistachioSelectedOption == 1 || PistachioSelectedOption == 2) {
    Serial.println("Pistachio After...");
    runDCMotor1Backward();
    PistachioSensorDetect();
  }
  runDCMotor1Forward();

  PressMachineSensorDetect();
  delay(2000);
  runDCMotor1Forward();
  delay(5000);
  stopDCMotor1();

  lcd.clear();
  lcd.print("Order is ready");
  displayMessage("Done!");
  delay(3000);
  
  lcd.clear();
  lcd.setCursor(0, 0);             
  lcd.print("1:Login");
  lcd.setCursor(0, 1);
  lcd.print("2:SignUp");

}


void getInput(char *buffer, const char *label, int maxLen) {
  lcd.clear();
  lcd.print(label);
  int index = 0;
  while (index < maxLen) {
    char key = keypad.getKey();
    if (key) {
      if (key == '.') break;
      buffer[index++] = key;
      lcd.setCursor(index, 1);
      lcd.print("*");
    }
  }
  buffer[index] = '\0';
}

// Signup function 
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
  lcd.clear();
  lcd.setCursor(0, 0);             
  lcd.print("1:Login");
  lcd.setCursor(0, 1);
  lcd.print("2:SignUp");
}

// Login function 
void login() {
  char username[USERNAME_LEN + 1];
  char password[PASSWORD_LEN + 1];
  char storedUser[USERNAME_LEN + 1];
  char storedPass[PASSWORD_LEN + 1];

  getInput(username, "Enter Username:", USERNAME_LEN);
  delay(500);
  getInput(password, "Enter Password:", PASSWORD_LEN);

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
  } else {
    lcd.clear();
    lcd.print("Access Denied");
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
  digitalWrite(dirPin, !direction); 
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
void turnRelayOn() {
digitalWrite(relayPin, LOW);
}

void turnRelayOff() {
digitalWrite(relayPin, HIGH);
}

void turnPumpRelayOn() {
digitalWrite(relayPumpPin, LOW);
}

void turnPumpRelayOff() {
digitalWrite(relayPumpPin, HIGH);
}

void PistachioSensorDetect() {
  while (true) {
    if (digitalRead(irSensor3Pin) == LOW) {
      delay(1000);
      stopDCMotor1();
      displayMessage("Running Pistachio Stepper...");
      runStepperPistachio(bistishioStepPin, bistishioDirPin, 80, HIGH);
      delay(2000);
      break;
     }
    }
}

void MixerSensorDetect() {
  while (true) {
    if (digitalRead(irSensor1Pin) == LOW) {
      delay(1250);
      stopDCMotor1();
      break;
     }
    }
}
void PressMachineSensorDetect() {
  while (true) {
    if (digitalRead(irSensor2Pin) == LOW) {
      delay(1000);
      stopDCMotor1();
      displayMessage("Running press machine DC motor...");
      runDCMotor2Forward();
      delay(4000);
      stopDCMotor2();
      break;
    }
  }
}

void RunMixing() {

  stopDCMotor1();
  delay(2000);

  displayMessage("Running relay Pump...");
  turnPumpRelayOn();
  delay(4000);
  turnPumpRelayOff();

  displayMessage("Running Milk Pump...");
  runPump();
  delay(5000);
  stopPump();

  displayMessage("Running Mixer Pump...");
  runMixerPump();
  delay(5000);
  stopMixerPump();

  displayMessage("Turning on Relay...");
  turnRelayOn();
  delay(8000);
  turnRelayOff();

  displayMessage("Running cleaning Pump...");
  runCleaningPump();
  delay(5000);
  stopCleaningPump();

  
}
void runStepperPistachio(int stepPin, int dirPin, int steps, bool direction) {
  // Move forward
  digitalWrite(dirPin, direction);
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
  }
  // Move back (reverse direction)
  digitalWrite(dirPin, !direction); // reverse the direction
  for (int i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepDelay);
  }
}
