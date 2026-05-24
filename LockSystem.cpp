#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Keypad.h>

#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

// ---------------- OLED ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------------- Keypad ----------------
const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ---------------- Fingerprint (R308) ----------------
// Arduino UNO: sensor connected via software serial
// Arduino RX = 10  <- Sensor TX
// Arduino TX = 11  -> Sensor RX
SoftwareSerial fingerSerial(10, 11);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

// ---------------- Password ----------------
String correctPassword = "2005";
String enteredPassword = "";

// ---------------- Settings ----------------
const uint8_t expectedFingerID = 1;     // Only fingerprint with ID = 1 is accepted
const uint8_t maxFingerTries  = 3;      // Maximum allowed fingerprint attempts
const uint16_t fingerTimeoutMs = 15000; // Fingerprint stage timeout (ms)

// ---------------- UI Helpers ----------------
void showMessage(const __FlashStringHelper* msg, uint8_t textSize = 2, int16_t x = 0, int16_t y = 20) {
  display.clearDisplay();
  display.setTextSize(textSize);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(x, y);
  display.print(msg);
  display.display();
}

void showStars(uint8_t count) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(10, 20);
  for (uint8_t i = 0; i < count; i++) display.print('*');
  display.display();
}


// Return values:
//  >=0  : fingerprint ID found
//  -3   : no finger on sensor
//  -2   : finger detected but no match found
//  -1   : sensor or processing error
int16_t readFingerIDOnce() {
  uint8_t p = finger.getImage();

  if (p == FINGERPRINT_NOFINGER) return -3;
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) return finger.fingerID;

  if (p == FINGERPRINT_NOTFOUND) return -2;

  return -1;
}

bool verifyFingerprintID(uint8_t targetID) {
  unsigned long start = millis();
  uint8_t tries = 0;

  while (millis() - start < fingerTimeoutMs && tries < maxFingerTries) {
    int16_t id = readFingerIDOnce();

    // No finger placed yet
    if (id == -3) {
      delay(50);
      continue;
    }

    // Sensor error
    if (id == -1) {
      showMessage(F("FP ERROR"), 2, 0, 20);
      delay(1200);
      tries++;
    }

    // Finger detected but no match
    if (id == -2) {
      showMessage(F("Access"), 2, 15, 10);
      display.setCursor(0, 35);
      display.print(F("Denied"));
      display.display();
      delay(1200);
      tries++;
    }

    // Fingerprint ID found
    if (id >= 0) {
      if ((uint8_t)id == targetID) return true;

      // Wrong fingerprint ID
      showMessage(F("Access"), 2, 15, 10);
      display.setCursor(0, 35);
      display.print(F("Denied"));
      display.display();
      delay(1200);
      tries++;
    }

    // Wait until finger is removed
    // (prevents reading the same finger continuously)
    while (finger.getImage() != FINGERPRINT_NOFINGER) {
      delay(50);
    }

    showMessage(F("TRY AGAIN"), 2, 0, 20);
    delay(700);
  }

  return false;
}


// ---------------- Setup & Loop ----------------
void setup() {
  // OLED initialization
  display.begin(0x3C, true);
  showMessage(F("Enter PIN"), 2, 0, 20);

  // Fingerprint sensor initialization
  fingerSerial.begin(57600);
  finger.begin(57600);

  // Initial sensor check (optional but useful)
  if (finger.verifyPassword()) {
    // Sensor is detected correctly
  } else {
    showMessage(F("FP ERROR"), 2, 0, 20);
    // Do not halt execution, but connection/baud rate may be wrong
    delay(2000);
    showMessage(F("Enter PIN"), 2, 0, 20);
  }
}

void loop() {
  char key = keypad.getKey();

  if (!key) return;

  // Accept digits only
  if (key >= '0' && key <= '9') {
    if (enteredPassword.length() < 4) {
      enteredPassword += key;
      showStars(enteredPassword.length());
    }
  }

  // '*' clears the entered PIN
  if (key == '*') {
    enteredPassword = "";
    showMessage(F("Enter PIN"), 2, 0, 20);
    return;
  }

  // When 4 digits are entered
  if (enteredPassword.length() == 4) {

    if (enteredPassword == correctPassword) {
      showMessage(F("PIN OK"), 2, 10, 20);
      delay(800);

      showMessage(F("Place"), 2, 20, 10);
      display.setCursor(0, 35);
      display.print(F("Finger ID1"));
      display.display();

      bool fingerOK = verifyFingerprintID(expectedFingerID);

      if (fingerOK) {
        // No servo used; only display unlock message
        showMessage(F("UNLOCKED"), 2, 0, 20);
        delay(5000);

        // After success, return to PIN entry
        enteredPassword = "";
        showMessage(F("Enter PIN"), 2, 0, 20);
        return;

      } else {
        // Fingerprint failed 3 times (based on maxFingerTries)
        showMessage(F("FP FAIL 3X"), 2, 0, 20);
        delay(1500);

        // Restart PIN entry
        enteredPassword = "";
        showMessage(F("Enter PIN"), 2, 0, 20);
        return;
      }

    } else {
      showMessage(F("NOT OK"), 2, 10, 20);
      delay(1500);

      showMessage(F("Wait 10s"), 2, 0, 20);
      delay(10000);
    }

    enteredPassword = "";
    showMessage(F("Enter PIN"), 2, 0, 20);
  }
}